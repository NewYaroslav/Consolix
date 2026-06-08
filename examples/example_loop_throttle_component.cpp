/// \example example_loop_throttle_component.cpp
/// \brief Demonstrates throttling the polling loop and waking it when work arrives.

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <consolix/core.hpp>

class WakeableQueueDemo final : public consolix::BaseLoopComponent {
public:
    explicit WakeableQueueDemo(
            std::shared_ptr<consolix::LoopThrottleComponent> throttle) :
        m_throttle(throttle) {
    }

    virtual ~WakeableQueueDemo() override {
        stop_producer();
    }

    bool on_once() override {
        CONSOLIX_STREAM() << "Starting producer thread.";
        m_producer = std::thread(&WakeableQueueDemo::producer_loop, this);
        return true;
    }

    void on_loop() override {
        int item = 0;
        while (try_pop(item)) {
            CONSOLIX_STREAM() << "Processed queued item: " << item;

            if (++m_processed >= 5) {
                CONSOLIX_STREAM() << "All queued items processed; stopping.";
                consolix::stop();
                return;
            }
        }
    }

    void on_shutdown(int exit_code) override {
        CONSOLIX_STREAM() << "Shutdown requested with code: " << exit_code;
        stop_producer();
    }

private:
    void producer_loop() {
        for (int item = 1; item <= 5 && !m_stop_producer.load(); ++item) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            if (m_stop_producer.load()) {
                return;
            }

            push(item);

            if (m_throttle) {
                m_throttle->wake();
            }
        }
    }

    void push(int item) {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_queue.push(item);
    }

    bool try_pop(int& item) {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        if (m_queue.empty()) {
            return false;
        }

        item = m_queue.front();
        m_queue.pop();
        return true;
    }

    void stop_producer() {
        m_stop_producer.store(true);
        if (m_throttle) {
            m_throttle->wake();
        }

        if (m_producer.joinable()) {
            m_producer.join();
        }
    }

    std::shared_ptr<consolix::LoopThrottleComponent> m_throttle;
    std::thread                                      m_producer;
    std::mutex                                       m_queue_mutex;
    std::queue<int>                                  m_queue;
    std::atomic<bool>                                m_stop_producer{false};
    int                                              m_processed{0};
};

int main() {
    consolix::add<consolix::LoggerComponent>();

    auto throttle = std::make_shared<consolix::LoopThrottleComponent>(
        std::chrono::milliseconds(500));

    consolix::add<WakeableQueueDemo>(throttle);

    // Put the throttle near the end so active components run before the wait.
    consolix::add(throttle);

    return consolix::run_for_exit_code();
}
