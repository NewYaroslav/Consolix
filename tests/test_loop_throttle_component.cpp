#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <consolix/core.hpp>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

long long elapsed_ms(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
}

void run_pre_wake_scenario() {
    consolix::AppComponentManager manager;
    auto throttle = manager.add<consolix::LoopThrottleComponent>(
        std::chrono::milliseconds(1000));

    expect(manager.initialize(), "LoopThrottleComponent did not initialize");

    throttle->wake();

    const auto start = std::chrono::steady_clock::now();
    manager.process();

    expect(elapsed_ms(start) < 500,
           "LoopThrottleComponent did not consume a pre-wake request");
}

void run_active_wake_scenario() {
    consolix::AppComponentManager manager;
    auto throttle = manager.add<consolix::LoopThrottleComponent>(
        std::chrono::milliseconds(1000));

    expect(manager.initialize(), "LoopThrottleComponent did not initialize");

    const auto start = std::chrono::steady_clock::now();
    std::thread worker([&manager]() {
        manager.process();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    throttle->wake();
    worker.join();

    expect(elapsed_ms(start) < 500,
           "LoopThrottleComponent wake did not end the active wait");
}

void run_set_delay_scenario() {
    consolix::AppComponentManager manager;
    auto throttle = manager.add<consolix::LoopThrottleComponent>(
        std::chrono::milliseconds(1000));

    expect(manager.initialize(), "LoopThrottleComponent did not initialize");

    throttle->set_delay(std::chrono::milliseconds(0));

    const auto start = std::chrono::steady_clock::now();
    manager.process();

    expect(elapsed_ms(start) < 500,
           "LoopThrottleComponent zero delay should not wait");
}

} // namespace

int main() {
    try {
        run_pre_wake_scenario();
        run_active_wake_scenario();
        run_set_delay_scenario();

        std::cout << "LoopThrottleComponent checks passed." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "LoopThrottleComponent test failed: " << e.what() << std::endl;
        return 1;
    }
}
