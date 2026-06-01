#include <iostream>
#include <stdexcept>
#include <vector>

#include <consolix/core.hpp>

namespace {

class ShutdownOrderComponent : public consolix::IAppComponent, public consolix::IShutdownable {
public:
    ShutdownOrderComponent(int id, std::vector<int>& shutdown_order) :
        m_id(id),
        m_shutdown_order(shutdown_order) {
    }

protected:
    bool initialize() override {
        m_initialized = true;
        return true;
    }

    bool is_initialized() const override {
        return m_initialized;
    }

    void process() override {
    }

    void shutdown(int signal) override {
        if (signal != 15) {
            throw std::runtime_error("Unexpected shutdown signal");
        }
        m_shutdown_order.push_back(m_id);
    }

private:
    int               m_id;
    std::vector<int>& m_shutdown_order;
    bool              m_initialized{false};
};

} // namespace

int main() {
    try {
        std::vector<int> shutdown_order;
        consolix::AppComponentManager manager;

        manager.add<ShutdownOrderComponent>(1, shutdown_order);
        manager.add<ShutdownOrderComponent>(2, shutdown_order);
        manager.add<ShutdownOrderComponent>(3, shutdown_order);

        if (!manager.initialize()) {
            throw std::runtime_error("Components were not initialized");
        }

        manager.shutdown(15);

        const std::vector<int> expected_order{3, 2, 1};
        if (shutdown_order != expected_order) {
            throw std::runtime_error("Shutdown order is not LIFO");
        }

        std::cout << "AppComponentManager shutdown order check passed." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "AppComponentManager shutdown order test failed: " << e.what() << std::endl;
        return 1;
    }
}
