#pragma once
#ifndef _CONSOLIX_JSON_UTILS_HPP_INCLUDED
#define _CONSOLIX_JSON_UTILS_HPP_INCLUDED

/// \file json_utils.hpp
/// \brief Utilities for working with JSON strings, including removing comments.

#include <string>
#include <algorithm>

namespace consolix {

    /// \brief Checks if a character in a JSON string is escaped.
    /// \param json_string The JSON string.
    /// \param quote_position The position of the character to check.
    /// \return `true` if the character is escaped, `false` otherwise.
    inline bool check_escaped(const std::string &json_string, size_t quote_position) {
        int backslash_count = 0;
        for (ptrdiff_t i = quote_position - 1; i >= 0 && json_string[i] == '\\'; --i) {
            ++backslash_count;
        }
        return (backslash_count % 2) != 0;
    };

    /// \brief Removes comments from a JSON string.
    ///
    /// This function processes a JSON string and removes comments, including
    /// single-line comments (e.g., `//` or `#`) and multi-line comments (e.g., `/* ... */`).
    /// The resulting JSON string may optionally retain whitespace or newlines
    /// where the comments were removed.
    ///
    /// \param json_string The JSON string to process.
    /// \param with_whitespace If `true`, comments are replaced with equivalent whitespace.
    ///                        If `false`, comments are removed without leaving whitespace.
    /// \param preserve_newlines If `true` and `with_whitespace` is enabled, newline characters
    ///                          in comments are preserved. Otherwise, all characters in the
    ///                          comments are replaced with whitespace.
    /// \return A JSON string with comments removed.
    std::string strip_json_comments(
            const std::string &json_string,
            bool with_whitespace = false,
            bool preserve_newlines = true) {
        enum CommentType { NO_COMMENT, SINGLE_COMMENT, MULTI_COMMENT };
        CommentType inside_comment = CommentType::NO_COMMENT;
        bool inside_string = false;

        size_t offset = 0;
        const size_t json_string_size = json_string.size();
        const size_t json_max_index = json_string_size - 1;
        std::string result;
        result.reserve(json_string_size);

        for (size_t i = 0; i < json_string_size; ++i) {
            const char current_character = json_string[i];
            const char next_character = i < json_max_index ? json_string[i + 1] : 0;

            if (inside_comment == CommentType::NO_COMMENT && current_character == '"') {
                if (!check_escaped(json_string, i)) inside_string = !inside_string;
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
                } else
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
                            i += 2;
                        } else {
                            result += '\n';
                            ++i;
                        }
                    } else {
                        i += (current_character == '\r') ? 2 : 1;
                    }
                    offset = i;
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
                            std::replace_if(temp.begin(), temp.end(), [](char ch){
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
            };
        }

        switch (inside_comment) {
        case CommentType::NO_COMMENT:
            result += json_string.substr(offset);
            break;
        case CommentType::SINGLE_COMMENT:
            if (with_whitespace) {
                result.append(std::string(json_max_index - offset, ' '));
            }
            break;
        case CommentType::MULTI_COMMENT:
            if (with_whitespace) {
                if (preserve_newlines) {
                    std::string temp = json_string.substr(offset, json_max_index - offset);
                    std::replace_if(temp.begin(), temp.end(), [](char ch){
                        return ch != '\n' && ch != '\r';
                    }, ' ');
                    result += temp;
                } else {
                    result.append(std::string(json_max_index - offset, ' '));
                }
            }
            break;
        default:
            break;
        };
        return result;
    };
} // namespace consolix

#endif // _CONSOLIX_JSON_UTILS_HPP_INCLUDED
