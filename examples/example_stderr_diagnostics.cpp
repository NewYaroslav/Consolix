/// \example example_stderr_diagnostics.cpp
/// \brief Demonstrates the five CONSOLIX_STDERR_* / CONSOLIX_LOG_* diagnostic macros.

#include <consolix/core.hpp>

/// \class DiagnosticDemo
/// \brief Demonstrates the five stderr / log diagnostic streams.
///
/// The example exercises:
///   - `CONSOLIX_STDERR_STREAM(level)` — stderr only.
///   - `CONSOLIX_STDERR_STREAM(level) << color(...)` — stderr with color.
///   - `CONSOLIX_STDERR_LOG_STREAM(level)` — stderr + `CONSOLIX_LOGIT_DEFAULT_BACKENDS`.
///   - `CONSOLIX_STDERR_LOG_STREAM_EX(level, ...)` — stderr + explicit backends.
///   - `CONSOLIX_LOG_STREAM(level)` — file logger + console + `CONSOLIX_LOGIT_DEFAULT_BACKENDS`.
///   - `CONSOLIX_LOG_STREAM_EX(level, ...)` — multi-target + explicit backends.
class DiagnosticDemo final : public consolix::BaseLoopComponent {
public:

    virtual ~DiagnosticDemo() override = default;

    /// \brief Called once at the start of the loop.
    /// \return `true` to signal successful initialization.
    bool on_once() override {
        CONSOLIX_STDERR_STREAM(logit::LogLevel::LOG_LVL_ERROR)
            << "[plain] hello from stderr (no color)";

        CONSOLIX_STDERR_STREAM(logit::LogLevel::LOG_LVL_ERROR)
            << consolix::color(consolix::TextColor::Red)
            << "[error] colored stderr via << color(...)";

#if CONSOLIX_USE_LOGIT == 1
        CONSOLIX_STDERR_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << "[log-stream] default backends (info)";

        CONSOLIX_STDERR_LOG_STREAM_EX(
                logit::LogLevel::LOG_LVL_WARN,
                CONSOLIX_LOGIT_LOGGER_INDEX, 0)
            << "[log-stream-ex] explicit backends (warn)";

        CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << "[multi] file+console+default backends (info)";

        CONSOLIX_LOG_STREAM_EX(logit::LogLevel::LOG_LVL_DEBUG, 0)
            << "[multi-ex] file+console+explicit backends (debug)";
#else
        consolix::StderrStream()
            << "[fallback] stderr-only — level/backend args are ignored when LogIt is disabled";
#endif

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
#if CONSOLIX_USE_LOGIT == 1
        CONSOLIX_STDERR_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << "shutdown: stderr diagnostic demo done";
#else
        consolix::StderrStream() << "shutdown: stderr diagnostic demo done";
#endif
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
