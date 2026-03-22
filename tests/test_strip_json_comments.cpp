#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <consolix/utils/json_utils.hpp>

#ifndef CONSOLIX_TEST_DATA_DIR
#error "CONSOLIX_TEST_DATA_DIR must point to the test fixture directory."
#endif

std::string read_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path);
    }

    return std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
}

std::string fixture_path(const char* file_name) {
    return std::string(CONSOLIX_TEST_DATA_DIR) + "/" + file_name;
}

int main() {
    try {
        const std::string input_file = fixture_path("test_input.json");
        const std::string json_string = read_file(input_file);

        struct TestCase {
            bool with_whitespace;
            bool preserve_newlines;
            const char* expected_file;
        };

        const std::array<TestCase, 4> test_cases{{
            {false, false, "test_output_no_whitespace_no_newlines.json"},
            {false, true,  "test_output_no_whitespace_preserve_newlines.json"},
            {true,  false, "test_output_whitespace_no_newlines.json"},
            {true,  true,  "test_output_whitespace_preserve_newlines.json"}
        }};

        for (const TestCase& test_case : test_cases) {
            const std::string actual = consolix::strip_json_comments(
                json_string,
                test_case.with_whitespace,
                test_case.preserve_newlines);

            const std::string expected_path = fixture_path(test_case.expected_file);
            const std::string expected = read_file(expected_path);

            if (actual != expected) {
                std::cerr
                    << "Mismatch for case "
                    << "with_whitespace=" << test_case.with_whitespace << ", "
                    << "preserve_newlines=" << test_case.preserve_newlines << ", "
                    << "expected_file=" << expected_path
                    << std::endl;
                return 1;
            }
        }

        std::cout << "All JSON comment stripping checks passed." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
