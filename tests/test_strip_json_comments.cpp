#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <consolix/utils/json_utils.hpp>
#include <consolix/utils/path_utils.hpp>

/// \brief Читает содержимое файла в строку.
/// \param file_path Путь к файлу.
/// \return Содержимое файла как строка.
/// \throws std::runtime_error Если файл не удалось открыть.
std::string read_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

/// \brief Записывает строку в файл.
/// \param file_path Путь к файлу.
/// \param content Строка для записи.
/// \throws std::runtime_error Если файл не удалось открыть.
void write_file(const std::string& file_path, const std::string& content) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to write to file: " + file_path);
    }
    file << content;
}

int main() {
    try {
        // Пути к файлам
        const std::string input_file = "test_input.json";
        const std::string output_file_base = "test_output";

        // Чтение входного файла
        std::string json_string = read_file(consolix::resolve_exec_path(input_file));

        // Проверка различных вариантов настроек
        for (bool with_whitespace : {false, true}) {
            for (bool preserve_newlines : {false, true}) {
                // Применяем функцию strip_json_comments
                std::string result = consolix::strip_json_comments(json_string, with_whitespace, preserve_newlines);

                // Формируем имя выходного файла
                std::string output_file = output_file_base +
                    (with_whitespace ? "_whitespace" : "_no_whitespace") +
                    (preserve_newlines ? "_preserve_newlines" : "_no_newlines") +
                    ".json";

                // Записываем результат в файл
                write_file(consolix::resolve_exec_path(output_file), result);

                // Отладочный вывод
                std::cout << "Processed with "
                          << "with_whitespace=" << with_whitespace << ", "
                          << "preserve_newlines=" << preserve_newlines << "\n"
                          << "Result saved to: " << output_file << std::endl;
            }
        }

        std::cout << "All tests completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
