// tests/test_log_stream_macros.cpp
//
// Verifies that CONSOLIX_LOG_STREAM and CONSOLIX_LOG_STREAM_EX keep the
// Consolix routing contract: display streams target the single-mode console
// and file logger by default, while plain LogIt broadcasts do not reach the
// console backend.

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <consolix/core.hpp>

namespace {

#if CONSOLIX_USE_LOGIT == 1

bool contains_message(const std::string& text, const std::string& marker) {
    return text.find(marker) != std::string::npos;
}

bool contains_message(const std::vector<std::string>& messages, const std::string& marker) {
    return std::find_if(
        messages.begin(),
        messages.end(),
        [&](const std::string& message) {
            return contains_message(message, marker);
        }) != messages.end();
}

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

void test_default_console_is_single_mode() {
    if (!LOGIT_IS_SINGLE_MODE(CONSOLIX_LOGIT_CONSOLE_INDEX)) {
        throw std::runtime_error("primary console logger is not single-mode");
    }
}

void test_general_logit_stream_does_not_hit_console() {
    std::ostringstream captured_cout;
    std::ostringstream captured_cerr;
    std::streambuf* old_cout = std::cout.rdbuf(captured_cout.rdbuf());
    std::streambuf* old_cerr = std::cerr.rdbuf(captured_cerr.rdbuf());

    try {
        LOGIT_STREAM_INFO() << "[broadcast-not-console]";
        LOGIT_WAIT();
    } catch (...) {
        std::cout.rdbuf(old_cout);
        std::cerr.rdbuf(old_cerr);
        throw;
    }

    std::cout.rdbuf(old_cout);
    std::cerr.rdbuf(old_cerr);

    if (contains_message(captured_cout.str(), "[broadcast-not-console]") ||
        contains_message(captured_cerr.str(), "[broadcast-not-console]")) {
        throw std::runtime_error("general LogIt broadcast reached the single-mode console");
    }
}

void test_consolix_log_stream_hits_console_by_level() {
    std::ostringstream captured_cout;
    std::ostringstream captured_cerr;
    std::streambuf* old_cout = std::cout.rdbuf(captured_cout.rdbuf());
    std::streambuf* old_cerr = std::cerr.rdbuf(captured_cerr.rdbuf());

    try {
        CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
            << "[consolix-info-console]";
        CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_ERROR)
            << "[consolix-error-console]";
        LOGIT_WAIT();
    } catch (...) {
        std::cout.rdbuf(old_cout);
        std::cerr.rdbuf(old_cerr);
        throw;
    }

    std::cout.rdbuf(old_cout);
    std::cerr.rdbuf(old_cerr);

    if (!contains_message(captured_cout.str(), "[consolix-info-console]")) {
        throw std::runtime_error("CONSOLIX_LOG_STREAM info did not reach stdout");
    }
    if (!contains_message(captured_cerr.str(), "[consolix-error-console]")) {
        throw std::runtime_error("CONSOLIX_LOG_STREAM error did not reach stderr");
    }
}

void test_consolix_log_stream_hits_file_logger() {
    CONSOLIX_LOG_STREAM(logit::LogLevel::LOG_LVL_INFO)
        << "[consolix-file-logger]";
    LOGIT_WAIT();

    const std::string path = LOGIT_GET_LAST_FILE_PATH(CONSOLIX_LOGIT_LOGGER_INDEX);
    if (path.empty()) {
        throw std::runtime_error("CONSOLIX_LOG_STREAM did not update the file logger path");
    }

    const logit::LogFileReadResult read_result =
        LOGIT_READ_LOG_FILE(CONSOLIX_LOGIT_LOGGER_INDEX, path);
    if (!read_result.ok ||
        !contains_message(read_result.content, "[consolix-file-logger]")) {
        throw std::runtime_error("CONSOLIX_LOG_STREAM did not reach the file logger");
    }
}

void test_consolix_log_stream_ex_keeps_defaults_and_adds_backends() {
    LOGIT_ADD_MEMORY_LOGGER_DEFAULT_SINGLE_MODE();
    const int memory_logger_index = static_cast<int>(
        logit::Logger::get_instance().logger_count() - 1);

    std::ostringstream captured_cout;
    std::streambuf* old_cout = std::cout.rdbuf(captured_cout.rdbuf());

    try {
        CONSOLIX_LOG_STREAM_EX(
                logit::LogLevel::LOG_LVL_WARN,
                memory_logger_index)
            << "[consolix-extra-backend]";
        LOGIT_WAIT();
    } catch (...) {
        std::cout.rdbuf(old_cout);
        throw;
    }

    std::cout.rdbuf(old_cout);

    if (!contains_message(captured_cout.str(), "[consolix-extra-backend]")) {
        throw std::runtime_error("CONSOLIX_LOG_STREAM_EX skipped the default console");
    }

    const auto buffered = LOGIT_GET_BUFFERED_STRINGS(memory_logger_index);
    if (!contains_message(buffered, "[consolix-extra-backend]")) {
        throw std::runtime_error("CONSOLIX_LOG_STREAM_EX skipped the explicit backend");
    }
}

#endif // CONSOLIX_USE_LOGIT == 1

} // namespace

int main() {
    try {
#if CONSOLIX_USE_LOGIT == 1
        consolix::LoggerComponent logger("%v", "%v", "%v", 1);
        (void)logger;
        test_default_console_is_single_mode();
        test_general_logit_stream_does_not_hit_console();
        test_consolix_log_stream_hits_console_by_level();
        test_consolix_log_stream_hits_file_logger();
        test_consolix_log_stream_ex_keeps_defaults_and_adds_backends();
        test_log_stream();
        test_log_stream_ex();
        LOGIT_WAIT();
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
