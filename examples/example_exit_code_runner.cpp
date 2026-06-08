/// \example example_exit_code_runner.cpp
/// \brief Demonstrates returning an exit code from the Consolix lifecycle runner.

#include <consolix/core.hpp>

class ExitCodeRunnerDemo final : public consolix::BaseLoopComponent {
public:
    bool on_once() override {
        CONSOLIX_STREAM() << "Exit-code runner demo initialized.";
        return true;
    }

    void on_loop() override {
        CONSOLIX_STREAM() << "Requesting stop with exit code 7.";
        consolix::stop(7);
    }

    void on_shutdown(int exit_code) override {
        CONSOLIX_STREAM() << "Shutdown completed with code: " << exit_code;
    }
};

int main() {
    consolix::add<ExitCodeRunnerDemo>();
    return consolix::run_for_exit_code();
}
