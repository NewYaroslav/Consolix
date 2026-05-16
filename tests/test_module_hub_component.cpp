#include <stdexcept>

#include <consolix/core.hpp>
#include <consolix/components.hpp>

#if CONSOLIX_USE_EVENT_HUB != 1
#error "test_module_hub_component requires CONSOLIX_USE_EVENT_HUB=1"
#endif

class TestModule final : public event_hub::Module {
public:
    explicit TestModule(event_hub::EventBus& bus, int& initialized, int& processed, int& shutdown) :
        event_hub::Module(bus),
        m_initialized(initialized),
        m_processed(processed),
        m_shutdown(shutdown) {
    }

protected:
    void on_initialize() override {
        ++m_initialized;
        tasks().post([this] {
            ++m_processed;
        });
    }

    std::size_t on_process() override {
        ++m_processed;
        return 1;
    }

    void on_shutdown() noexcept override {
        ++m_shutdown;
    }

private:
    int& m_initialized;
    int& m_processed;
    int& m_shutdown;
};

int main() {
    int initialized = 0;
    int processed = 0;
    int shutdown = 0;

    event_hub::ModuleHub hub;
    hub.emplace_module<TestModule>(initialized, processed, shutdown);

    consolix::AppComponentManager manager;
    auto component = manager.add<consolix::ModuleHubComponent>(&hub);

    if (!manager.initialize()) {
        throw std::runtime_error("ModuleHubComponent failed to initialize");
    }
    if (initialized != 1) {
        throw std::runtime_error("ModuleHubComponent did not initialize ModuleHub");
    }

    manager.process();

    if (processed < 2) {
        throw std::runtime_error("ModuleHubComponent did not process ModuleHub work");
    }
    if (component->last_work_count() == 0) {
        throw std::runtime_error("ModuleHubComponent did not report processed work");
    }

    manager.shutdown(0);

    if (shutdown != 1) {
        throw std::runtime_error("ModuleHubComponent did not shutdown ModuleHub exactly once");
    }

    return 0;
}
