/// \example example_shutdown_and_resources.cpp
/// \brief Demonstrates safe resource cleanup and worker-thread shutdown in Consolix.

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <consolix/core.hpp>

class WorkerResource {
public:
    explicit WorkerResource(std::string name) :
        m_name(std::move(name)) {
        std::cout << "Acquired resource: " << m_name << std::endl;
    }

    ~WorkerResource() {
        std::cout << "Released resource: " << m_name << std::endl;
    }

private:
    std::string m_name;
};

class ShutdownAndResourcesDemo final : public consolix::BaseLoopComponent {
public:
    virtual ~ShutdownAndResourcesDemo() override {
        request_stop_and_join();
    }

    bool on_once() override {
        m_resource.reset(new WorkerResource("background-worker"));
        m_worker = std::thread(&ShutdownAndResourcesDemo::worker_loop, this);

        std::cout << "Worker thread started." << std::endl;
        std::cout << "The demo will stop itself after a few loop iterations." << std::endl;
        std::cout << "If the process receives SIGINT/SIGTERM on POSIX, the same on_shutdown() path is used later in normal application code." << std::endl;
        return true;
    }

    void on_loop() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        const int tick = ++m_tick_count;
        std::cout
            << "Main loop tick " << tick
            << ", worker heartbeats: "
            << m_heartbeats.load()
            << std::endl;

        if (tick >= 5) {
            std::cout << "Requesting cooperative stop via consolix::stop()." << std::endl;
            consolix::stop();
        }
    }

    void on_shutdown(int signal) override {
        std::cout << "Shutdown requested with signal code: " << signal << std::endl;
        request_stop_and_join();

        m_resource.reset();
        std::cout << "Worker joined and owned resource released in on_shutdown()." << std::endl;
    }

private:
    void request_stop_and_join() {
        m_worker_stop.store(true);
        if (m_worker.joinable()) {
            m_worker.join();
        }
    }

    void worker_loop() {
        while (!m_worker_stop.load()) {
            ++m_heartbeats;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    std::thread                  m_worker;
    std::unique_ptr<WorkerResource> m_resource;
    std::atomic<bool>            m_worker_stop{false};
    std::atomic<unsigned>        m_heartbeats{0};
    std::atomic<int>             m_tick_count{0};
};

int main() {
    consolix::add<ShutdownAndResourcesDemo>();
    consolix::run();
    return 0;
}
