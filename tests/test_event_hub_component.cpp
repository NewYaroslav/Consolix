#include <memory>
#include <stdexcept>

#include <consolix/core.hpp>
#include <consolix/components.hpp>

#if CONSOLIX_USE_EVENT_HUB != 1
#error "test_event_hub_component requires CONSOLIX_USE_EVENT_HUB=1"
#endif

struct TestEvent {
    int value;
};

int main() {
    event_hub::EventBus bus;
    event_hub::TaskManager tasks;

    int event_sum = 0;
    bool task_ran = false;

    event_hub::EventEndpoint endpoint(bus);
    endpoint.subscribe<TestEvent>(event_hub::DeliveryPolicy::queued, [&event_sum](const TestEvent& event) {
        event_sum += event.value;
    });

    endpoint.post<TestEvent>(7);
    tasks.post([&task_ran] {
        task_ran = true;
    });

    consolix::AppComponentManager manager;
    auto component = manager.add<consolix::EventHubComponent>(&bus, &tasks);

    if (!manager.initialize()) {
        throw std::runtime_error("EventHubComponent failed to initialize");
    }

    manager.process();

    if (event_sum != 7) {
        throw std::runtime_error("EventHubComponent did not process EventBus queue");
    }
    if (!task_ran) {
        throw std::runtime_error("EventHubComponent did not process TaskManager queue");
    }
    if (component->last_work_count() != 2) {
        throw std::runtime_error("Unexpected EventHubComponent work count");
    }

    endpoint.post<TestEvent>(5);
    tasks.post([] {});
    manager.shutdown(0);
    manager.process();

    if (event_sum != 7) {
        throw std::runtime_error("EventHubComponent did not clear pending events during shutdown");
    }

    return 0;
}
