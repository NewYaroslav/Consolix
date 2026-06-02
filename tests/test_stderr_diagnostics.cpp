// tests/test_stderr_diagnostics.cpp
//
// Verifies that the diagnostic stream macro matrix (CONSOLIX_STDERR_STREAM,
// CONSOLIX_STDERR_LOG_STREAM, CONSOLIX_STDERR_LOG_STREAM_EX,
// CONSOLIX_LOG_STREAM, CONSOLIX_LOG_STREAM_EX) compiles and behaves as
// documented: stderr-side macros route their output to std::cerr, while
// the multi-target LOG_* macros only need to compile and execute without
// throwing (their primary targets are file/console loggers, not stderr).

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <consolix/core.hpp>
#include <consolix/components.hpp>

namespace {

void require_contains(const std::string& haystack,
                      const std::string& needle,
                      const char* context) {
    if (haystack.find(needle) == std::string::npos) {
        throw std::runtime_error(
            std::string(context) +
            ": expected substring '" + needle +
            "' in captured output, got: '" + haystack + "'");
    }
}

// RAII guard that swaps std::cerr's streambuf for the duration of the scope.
class CerrRedirect {
public:
    explicit CerrRedirect(std::stringstream& sink) :
        m_old_buf(std::cerr.rdbuf(sink.rdbuf())) {}

    ~CerrRedirect() {
        std::cerr.rdbuf(m_old_buf);
    }

    CerrRedirect(const CerrRedirect&) = delete;
    CerrRedirect& operator=(const CerrRedirect&) = delete;

private:
    std::streambuf* m_old_buf;
};

#if CONSOLIX_USE_LOGIT == 1

void test_stderr_stream_plain() {
    std::stringstream captured;
    {
        CerrRedirect redirect(captured);
        CONSOLIX_STDERR_STREAM(logit::LogLevel::LOG_LVL_ERROR)
            << "[stderr-only]" << std::endl;
    }

    const std::string out = captured.str();
    require_contains(out, "[stderr-only]",
                     "CONSOLIX_STDERR_STREAM (plain)");
    if (out.find('\n') == std::string::npos) {
        throw std::runtime_error(
            "CONSOLIX_STDERR_STREAM (plain): missing trailing newline");
    }
    if (out.find("\033[") != std::string::npos) {
        throw std::runtime_error(
            "CONSOLIX_STDERR_STREAM (plain): unexpected ANSI escape code "
            "in uncolored output");
    }
}

void test_stderr_stream_colored() {
    std::stringstream captured;
    {
        CerrRedirect redirect(captured);
        CONSOLIX_STDERR_STREAM(logit::LogLevel::LOG_LVL_ERROR)
            << "[stderr-colored]x"
            << consolix::color(consolix::TextColor::Red)
            << "y";
    }

    const std::string out = captured.str();
    require_contains(out, "[stderr-colored]x",
                     "CONSOLIX_STDERR_STREAM (colored)");
    require_contains(out, "y",
                     "CONSOLIX_STDERR_STREAM (colored)");
    require_contains(out, "\033[",
                     "CONSOLIX_STDERR_STREAM (colored)");
}

void test_stderr_log_stream_default() {
    std::stringstream captured;
    {
        CerrRedirect redirect(captured);
        CONSOLIX_STDERR_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << "[stderr-log-default]" << std::endl;
    }

    require_contains(captured.str(), "[stderr-log-default]",
                     "CONSOLIX_STDERR_LOG_STREAM");
}

void test_stderr_log_stream_ex() {
    std::stringstream captured;
    {
        CerrRedirect redirect(captured);
        CONSOLIX_STDERR_LOG_STREAM_EX(logit::LogLevel::LOG_LVL_WARN, 0)
            << "[stderr-log-ex]" << std::endl;
    }

    require_contains(captured.str(), "[stderr-log-ex]",
                     "CONSOLIX_STDERR_LOG_STREAM_EX");
}

void test_log_stream_default() {
    try {
        CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_DEBUG)
            << "[log-default]" << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("CONSOLIX_LOG_STREAM threw: ") + e.what());
    } catch (...) {
        throw std::runtime_error(
            "CONSOLIX_LOG_STREAM threw an unknown exception");
    }
}

void test_log_stream_ex() {
    try {
        CONSOLIX_LOG_STREAM_EX(logit::LogLevel::LOG_LVL_INFO, 0)
            << "[log-ex]" << std::endl;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("CONSOLIX_LOG_STREAM_EX threw: ") + e.what());
    } catch (...) {
        throw std::runtime_error(
            "CONSOLIX_LOG_STREAM_EX threw an unknown exception");
    }
}

#endif // CONSOLIX_USE_LOGIT == 1

} // namespace

int main() {
    try {
#if CONSOLIX_USE_LOGIT == 1
        test_stderr_stream_plain();
        test_stderr_stream_colored();
        test_stderr_log_stream_default();
        test_stderr_log_stream_ex();
        test_log_stream_default();
        test_log_stream_ex();
        std::cout << "Diagnostic stream macro checks passed." << std::endl;
#else
        // Without LogIt the macros expand to no-arg constructors; the
        // runtime semantics are no-op fallbacks, but the test still has
        // to make sure each macro can be instantiated and streamed into.
        CONSOLIX_STDERR_STREAM(0)            << "[stderr-only]";
        CONSOLIX_STDERR_LOG_STREAM(0)        << "[stderr-log-default]";
        CONSOLIX_STDERR_LOG_STREAM_EX(0, 0)  << "[stderr-log-ex]";
        CONSOLIX_LOG_STREAM(0)               << "[log-default]";
        CONSOLIX_LOG_STREAM_EX(0, 0)         << "[log-ex]";
        std::cout << "Diagnostic stream macro compile-time checks passed."
                  << std::endl;
#endif
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Diagnostic stream test failed: " << e.what()
                  << std::endl;
        return 1;
    }
}
