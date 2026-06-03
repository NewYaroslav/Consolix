/// \example example_stderr_diagnostics.cpp
/// \brief Demonstrates the CONSOLIX_LOG_STREAM / CONSOLIX_LOG_STREAM_EX
///        multi-target diagnostic macros.

#include <consolix/core.hpp>

/// \class DiagnosticDemo
/// \brief Demonstrates the multi-target diagnostic streams.
///
/// The example exercises:
///   - `CONSOLIX_LOG_STREAM(level)` — file logger + console.
///   - `CONSOLIX_LOG_STREAM(level) << color(...)` — multi-target with color.
///   - `CONSOLIX_LOG_STREAM_EX(level, ...)` — multi-target + explicit backends.
class DiagnosticDemo final : public consolix::BaseLoopComponent {
public:

    virtual ~DiagnosticDemo() override = default;

    /// \brief Called once at the start of the loop.
    /// \return `true` to signal successful initialization.
    bool on_once() override {
        CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << "[multi] file+console (info)";

        CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << consolix::color(consolix::TextColor::Red)
            << "[multi-colored] file+console with color";

        CONSOLIX_LOG_STREAM_EX(
                logit::LogLevel::LOG_LVL_WARN,
                CONSOLIX_LOGIT_LOGGER_INDEX, 0)
            << "[multi-ex] file+console+explicit backends (warn)";

        return true;
    }

    /// \brief Called repeatedly during the main loop.
    void on_loop() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (++m_tick >= 1) {
            consolix::stop();
        }
    }

    /// \brief Called during application shutdown after a stop request or termination signal.
    /// \param signal The shutdown signal.
    void on_shutdown(int /*signal*/) override {
        CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << "shutdown: multi-target diagnostic demo done";
    }

private:
    unsigned m_tick{0}; ///< Loop tick counter used to request a single iteration.
};

/// \brief Main entry point of the program.
/// \return Process exit code.
int main() {
    consolix::add<DiagnosticDemo>();
    consolix::run();
    return 0;
}
