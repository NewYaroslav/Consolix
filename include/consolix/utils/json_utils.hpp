#pragma once
#ifndef _CONSOLIX_JSON_UTILS_HPP_INCLUDED
#define _CONSOLIX_JSON_UTILS_HPP_INCLUDED

/// \file json_utils.hpp
/// \brief Utilities for working with JSON strings, including removing comments.

#include <algorithm>
#include <cstddef>
#include <string>

namespace consolix {

    /// \brief Checks if a character in a JSON string is escaped.
    /// \param json_string The JSON string.
    /// \param quote_position The position of the character to check.
    /// \return `true` if the character is escaped, `false` otherwise.
    inline bool check_escaped(const std::string& json_string, std::size_t quote_position) {
        if (quote_position == 0) {
            return false;
        }

        int backslash_count = 0;
        for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(quote_position) - 1;
                i >= 0 && json_string[static_cast<std::size_t>(i)] == '\\';
                --i) {
            ++backslash_count;
        }

        return (backslash_count % 2) != 0;
    }

    /// \brief Removes comments from a JSON string.
    /// \param json_string The JSON string to process.
    /// \param with_whitespace If `true`, comments are replaced with equivalent whitespace.
    /// \param preserve_newlines If `true` and `with_whitespace` is enabled, newline characters are preserved.
    /// \return A JSON string with comments removed.
    inline std::string strip_json_comments(
            const std::string& json_string,
            bool with_whitespace = false,
            bool preserve_newlines = true) {
        if (json_string.empty()) {
            return std::string();
        }

        enum CommentType { NO_COMMENT, SINGLE_COMMENT, MULTI_COMMENT };
        CommentType inside_comment = CommentType::NO_COMMENT;
        bool inside_string = false;

        std::size_t offset = 0;
        const std::size_t json_string_size = json_string.size();
        const std::size_t json_max_index = json_string_size - 1;
        std::string result;
        result.reserve(json_string_size);

        for (std::size_t i = 0; i < json_string_size; ++i) {
            const char current_character = json_string[i];
            const char next_character = i < json_max_index ? json_string[i + 1] : 0;

            if (inside_comment == CommentType::NO_COMMENT && current_character == '"') {
                if (!check_escaped(json_string, i)) {
                    inside_string = !inside_string;
                }
            }

            if (inside_string) continue;

            switch (inside_comment) {
            case CommentType::NO_COMMENT:
                if (current_character == '#' || (current_character == '/' && next_character == '/')) {
                    inside_comment = SINGLE_COMMENT;
                    result.append(json_string, offset, i - offset);
                    offset = i;
                    i += (next_character == '/') ? 2 : 1;
                    continue;
                }
                if (current_character == '/' && next_character == '*') {
                    inside_comment = MULTI_COMMENT;
                    result.append(json_string, offset, i - offset);
                    offset = i;
                    ++i;
                    continue;
                }
                break;
            case CommentType::SINGLE_COMMENT:
                if ((current_character == '\r' && next_character == '\n') || current_character == '\n') {
                    inside_comment = NO_COMMENT;
                    if (with_whitespace) {
                        result.append(std::string(i - offset, ' '));
                        if (current_character == '\r') {
                            result += "\r\n";
                            ++i;
                        } else {
                            result += '\n';
                        }
                    } else {
                        if (current_character == '\r') {
                            ++i;
                        }
                    }
                    offset = i + 1;
                    continue;
                }
                break;
            case CommentType::MULTI_COMMENT:
                if (current_character == '*' && next_character == '/') {
                    inside_comment = CommentType::NO_COMMENT;
                    ++i;
                    if (with_whitespace) {
                        if (preserve_newlines) {
                            std::string temp = json_string.substr(offset, i - offset + 1);
                            std::replace_if(temp.begin(), temp.end(), [](char ch) {
                                return ch != '\n' && ch != '\r';
                            }, ' ');
                            result += temp;
                        } else {
                            result.append(std::string(i - offset + 1, ' '));
                        }
                    }
                    offset = i + 1;
                    continue;
                }
                break;
            default:
                break;
            }
        }

        switch (inside_comment) {
        case CommentType::NO_COMMENT:
            result += json_string.substr(offset);
            break;
        case CommentType::SINGLE_COMMENT:
            if (with_whitespace) {
                result.append(std::string(json_string_size - offset, ' '));
            }
            break;
        case CommentType::MULTI_COMMENT:
            if (with_whitespace) {
                if (preserve_newlines) {
                    std::string temp = json_string.substr(offset, json_string_size - offset);
                    std::replace_if(temp.begin(), temp.end(), [](char ch) {
                        return ch != '\n' && ch != '\r';
                    }, ' ');
                    result += temp;
                } else {
                    result.append(std::string(json_string_size - offset, ' '));
                }
            }
            break;
        default:
            break;
        }

        return result;
    }
} // namespace consolix

#endif // _CONSOLIX_JSON_UTILS_HPP_INCLUDED
