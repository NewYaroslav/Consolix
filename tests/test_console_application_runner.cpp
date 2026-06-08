#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#include <consolix/core.hpp>

namespace {

struct ScenarioState {
    int initialize_calls = 0;
    int process_calls = 0;
    int shutdown_calls = 0;
    int shutdown_code = 0;
};

struct CountingService {
    explicit CountingService(int& destroyed) :
        destroyed_count(&destroyed) {
    }

    ~CountingService() {
        ++(*destroyed_count);
    }

    int* destroyed_count;
};

void expect(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

long long elapsed_ms(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
}

class NormalStopComponent final : public consolix::BaseLoopComponent {
public:
    explicit NormalStopComponent(ScenarioState& state) :
        m_state(state) {
    }

    bool on_once() override {
        ++m_state.initialize_calls;
        return true;
    }

    void on_loop() override {
        ++m_state.process_calls;
        consolix::stop();
    }

    void on_shutdown(int signal) override {
        ++m_state.shutdown_calls;
        m_state.shutdown_code = signal;
    }

private:
    ScenarioState& m_state;
};

class ExitCodeStopComponent final : public consolix::BaseLoopComponent {
public:
    ExitCodeStopComponent(ScenarioState& state, int exit_code) :
        m_state(state),
        m_exit_code(exit_code) {
    }

    bool on_once() override {
        ++m_state.initialize_calls;
        return true;
    }

    void on_loop() override {
        ++m_state.process_calls;
        consolix::stop(m_exit_code);
    }

    void on_shutdown(int signal) override {
        ++m_state.shutdown_calls;
        m_state.shutdown_code = signal;
    }

private:
    ScenarioState& m_state;
    int            m_exit_code;
};

class SignalStopComponent final : public consolix::BaseLoopComponent {
public:
    explicit SignalStopComponent(ScenarioState& state) :
        m_state(state) {
    }

    bool on_once() override {
        ++m_state.initialize_calls;
        return true;
    }

    void on_loop() override {
        ++m_state.process_calls;
        if (std::raise(SIGTERM) != 0) {
            throw std::runtime_error("std::raise(SIGTERM) failed");
        }
    }

    void on_shutdown(int signal) override {
        ++m_state.shutdown_calls;
        m_state.shutdown_code = signal;
    }

private:
    ScenarioState& m_state;
};

class ThrowingComponent final : public consolix::BaseLoopComponent {
public:
    explicit ThrowingComponent(ScenarioState& state) :
        m_state(state) {
    }

    bool on_once() override {
        ++m_state.initialize_calls;
        return true;
    }

    void on_loop() override {
        ++m_state.process_calls;
        throw std::runtime_error("intentional process failure");
    }

    void on_shutdown(int signal) override {
        ++m_state.shutdown_calls;
        m_state.shutdown_code = signal;
    }

private:
    ScenarioState& m_state;
};

class ForcedStopComponent final : public consolix::BaseLoopComponent {
public:
    ForcedStopComponent(
            ScenarioState& state,
            std::atomic<bool>& ready,
            std::thread::id& shutdown_thread_id) :
        m_state(state),
        m_ready(ready),
        m_shutdown_thread_id(shutdown_thread_id) {
    }

    bool on_once() override {
        ++m_state.initialize_calls;
        m_ready.store(true);
        return true;
    }

    void on_loop() override {
        ++m_state.process_calls;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    void on_shutdown(int signal) override {
        ++m_state.shutdown_calls;
        m_state.shutdown_code = signal;
        m_shutdown_thread_id = std::this_thread::get_id();
    }

private:
    ScenarioState&     m_state;
    std::atomic<bool>& m_ready;
    std::thread::id&   m_shutdown_thread_id;
};

class BlockingProcessComponent final : public consolix::BaseLoopComponent {
public:
    BlockingProcessComponent(
            ScenarioState& state,
            std::atomic<bool>& process_entered,
            std::atomic<bool>& release_process) :
        m_state(state),
        m_process_entered(process_entered),
        m_release_process(release_process) {
    }

    bool on_once() override {
        ++m_state.initialize_calls;
        return true;
    }

    void on_loop() override {
        ++m_state.process_calls;
        m_process_entered.store(true);
        while (!m_release_process.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void on_shutdown(int signal) override {
        ++m_state.shutdown_calls;
        m_state.shutdown_code = signal;
    }

private:
    ScenarioState&     m_state;
    std::atomic<bool>& m_process_entered;
    std::atomic<bool>& m_release_process;
};

class ThreadStopComponent final : public consolix::BaseLoopComponent {
public:
    ThreadStopComponent(ScenarioState& state, int exit_code) :
        m_state(state),
        m_exit_code(exit_code) {
    }

    virtual ~ThreadStopComponent() override {
        join_stop_thread();
    }

    bool on_once() override {
        ++m_state.initialize_calls;
        m_stop_thread = std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            consolix::stop(m_exit_code);
        });
        return true;
    }

    void on_loop() override {
        ++m_state.process_calls;
    }

    void on_shutdown(int signal) override {
        ++m_state.shutdown_calls;
        m_state.shutdown_code = signal;
        join_stop_thread();
    }

private:
    void join_stop_thread() {
        if (m_stop_thread.joinable()) {
            m_stop_thread.join();
        }
    }

    ScenarioState& m_state;
    int            m_exit_code;
    std::thread    m_stop_thread;
};

void run_normal_stop_scenario() {
    ScenarioState state;
    int service_destroyed = 0;

    consolix::ServiceLocator::get_instance().register_service<CountingService>(
        [&service_destroyed]() {
            return std::make_shared<CountingService>(service_destroyed);
        });

    consolix::AppComponentManager manager;
    manager.add<NormalStopComponent>(state);

    consolix::ConsoleApplicationRunner runner(manager);
    const int exit_code = runner.run_for_exit_code();

    expect(exit_code == 0, "normal stop must return 0");
    expect(state.initialize_calls == 1, "normal stop must initialize once");
    expect(state.process_calls == 1, "normal stop must process once");
    expect(state.shutdown_calls == 1, "normal stop must shutdown once");
    expect(state.shutdown_code == 0, "normal stop must shutdown with code 0");
    expect(!consolix::ServiceLocator::get_instance().has_service<CountingService>(),
           "runner must clear services");
    expect(service_destroyed == 1, "runner must clear services exactly once");
}

void run_runner_single_use_scenario() {
    ScenarioState state;
    consolix::AppComponentManager manager;
    manager.add<NormalStopComponent>(state);

    consolix::ConsoleApplicationRunner runner(manager);
    const int exit_code = runner.run_for_exit_code();

    expect(exit_code == 0, "single-use runner first run must return 0");

    bool threw = false;
    try {
        (void)runner.run_for_exit_code();
    } catch (const std::logic_error&) {
        threw = true;
    }

    expect(threw, "runner must reject a second run");
}

void run_component_exit_code_scenario() {
    ScenarioState state;
    consolix::AppComponentManager manager;
    manager.add<ExitCodeStopComponent>(state, 42);

    consolix::ConsoleApplicationRunner runner(manager);
    const int exit_code = runner.run_for_exit_code();

    expect(exit_code == 42, "stop(int) must return requested code");
    expect(state.shutdown_calls == 1, "stop(int) must shutdown once");
    expect(state.shutdown_code == 42, "stop(int) must pass code to shutdown");
}

void run_signal_stop_scenario() {
    ScenarioState state;
    consolix::AppComponentManager manager;
    manager.add<SignalStopComponent>(state);

    consolix::ConsoleApplicationRunner runner(manager);
    const int exit_code = runner.run_for_exit_code();

    expect(exit_code == SIGTERM, "signal stop must return signal code");
    expect(state.shutdown_calls == 1, "signal stop must shutdown once");
    expect(state.shutdown_code == SIGTERM, "signal stop must pass signal to shutdown");
}

void run_exception_scenario() {
    ScenarioState state;
    consolix::AppComponentManager manager;
    manager.add<ThrowingComponent>(state);

    consolix::ConsoleApplicationRunner runner(manager);
    const int exit_code = runner.run_for_exit_code();

    expect(exit_code != 0, "exception path must return a non-zero exit code");
    expect(state.shutdown_calls == 1, "exception path must shutdown once");
    expect(state.shutdown_code != 0, "exception path must pass non-zero code to shutdown");
}

void run_forced_stop_scenario() {
    ScenarioState state;
    std::atomic<bool> ready(false);
    std::thread::id runner_thread_id;
    std::thread::id shutdown_thread_id;
    int runner_exit_code = -1;

    consolix::AppComponentManager manager;
    manager.add<ForcedStopComponent>(state, ready, shutdown_thread_id);

    consolix::ConsoleApplicationRunner runner(manager);
    std::thread runner_thread([&]() {
        runner_thread_id = std::this_thread::get_id();
        runner_exit_code = runner.run_for_exit_code();
    });

    while (!ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    const bool completed = runner.request_forced_stop(
        24,
        std::chrono::milliseconds(2000));

    runner_thread.join();

    expect(completed, "forced stop must wait for runner cleanup");
    expect(runner_exit_code == 24, "forced stop must return requested code");
    expect(state.shutdown_calls == 1, "forced stop must shutdown once");
    expect(state.shutdown_code == 24, "forced stop must pass code to shutdown");
    expect(shutdown_thread_id == runner_thread_id,
           "forced stop cleanup must run on runner thread");
    expect(shutdown_thread_id != std::this_thread::get_id(),
           "forced stop cleanup must not run on requester thread");
}

void run_forced_stop_timeout_scenario() {
    ScenarioState state;
    std::atomic<bool> process_entered(false);
    std::atomic<bool> release_process(false);
    int runner_exit_code = -1;

    consolix::AppComponentManager manager;
    manager.add<BlockingProcessComponent>(state, process_entered, release_process);

    consolix::ConsoleApplicationRunner runner(manager);
    std::thread runner_thread([&]() {
        runner_exit_code = runner.run_for_exit_code();
    });

    while (!process_entered.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    const bool completed = runner.request_forced_stop(
        33,
        std::chrono::milliseconds(20));

    expect(!completed, "forced stop must report timeout while process is blocked");

    release_process.store(true);
    runner_thread.join();

    expect(runner_exit_code == 33, "timed-out forced stop must still request code");
    expect(state.shutdown_calls == 1, "timed-out forced stop must later shutdown once");
    expect(state.shutdown_code == 33, "timed-out forced stop must pass code to shutdown");
}

void run_stop_wakes_throttle_scenario() {
    ScenarioState state;
    consolix::AppComponentManager manager;
    manager.add<ThreadStopComponent>(state, 77);
    manager.add<consolix::LoopThrottleComponent>(std::chrono::milliseconds(1000));

    consolix::ConsoleApplicationRunner runner(manager);
    const auto start = std::chrono::steady_clock::now();
    const int exit_code = runner.run_for_exit_code();

    expect(exit_code == 77, "thread stop must return requested code");
    expect(elapsed_ms(start) < 500,
           "stop request must wake LoopThrottleComponent instead of waiting for delay");
    expect(state.shutdown_calls == 1, "thread stop must shutdown once");
    expect(state.shutdown_code == 77, "thread stop must pass code to shutdown");
}

} // namespace

int main() {
    try {
        run_normal_stop_scenario();
        run_runner_single_use_scenario();
        run_component_exit_code_scenario();
        run_signal_stop_scenario();
        run_exception_scenario();
        run_forced_stop_scenario();
        run_forced_stop_timeout_scenario();
        run_stop_wakes_throttle_scenario();

        std::cout << "ConsoleApplicationRunner checks passed." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ConsoleApplicationRunner test failed: " << e.what() << std::endl;
        return 1;
    }
}
