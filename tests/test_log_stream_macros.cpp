// tests/test_log_stream_macros.cpp
//
// Verifies that CONSOLIX_LOG_STREAM and CONSOLIX_LOG_STREAM_EX compile
// and execute without throwing. Actual log routing is covered by LogIt
// itself; here we only do a smoke-test.

#include <iostream>
#include <stdexcept>

#include <consolix/core.hpp>

namespace {

#if CONSOLIX_USE_LOGIT == 1

void test_log_stream() {
    try {
        CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << "[log-stream] smoke test";
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("CONSOLIX_LOG_STREAM threw: ") + e.what());
    } catch (...) {
        throw std::runtime_error("CONSOLIX_LOG_STREAM threw unknown exception");
    }
}

void test_log_stream_ex() {
    try {
        CONSOLIX_LOG_STREAM_EX(logit::LogLevel::LOG_LVL_WARN, 5)
            << "[log-stream-ex] smoke test";
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("CONSOLIX_LOG_STREAM_EX threw: ") + e.what());
    } catch (...) {
        throw std::runtime_error("CONSOLIX_LOG_STREAM_EX threw unknown exception");
    }
}

#endif // CONSOLIX_USE_LOGIT == 1

} // namespace

int main() {
    try {
#if CONSOLIX_USE_LOGIT == 1
        test_log_stream();
        test_log_stream_ex();
        std::cout << "Log stream macro checks passed." << std::endl;
#else
        CONSOLIX_LOG_STREAM(0) << "[fallback]";
        CONSOLIX_LOG_STREAM_EX(0, 0) << "[fallback-ex]";
        std::cout << "Log stream macro compile-time checks passed." << std::endl;
#endif
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Log stream test failed: " << e.what() << std::endl;
        return 1;
    }
}
