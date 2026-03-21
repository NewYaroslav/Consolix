#pragma once
#ifndef _CONSOLIX_STD_COMPAT_HPP_INCLUDED
#define _CONSOLIX_STD_COMPAT_HPP_INCLUDED

/// \file std_compat.hpp
/// \brief Compatibility helpers for standard library features used across Consolix.

#include <mutex>
#include <system_error>

#if __cplusplus >= 201703L
#include <filesystem>
#include <shared_mutex>
#elif __cplusplus >= 201402L
#include <experimental/filesystem>
#include <shared_mutex>
#else
#include <experimental/filesystem>
#endif

namespace consolix {
namespace compat {

#if __cplusplus >= 201703L

    namespace filesystem = std::filesystem;
    using shared_mutex = std::shared_mutex;

    template <typename Mutex>
    using shared_lock = std::shared_lock<Mutex>;

    inline filesystem::path relative(
            const filesystem::path& path,
            const filesystem::path& base,
            std::error_code& ec) {
        return filesystem::relative(path, base, ec);
    }

#elif __cplusplus >= 201402L

    namespace filesystem = std::experimental::filesystem;
    using shared_mutex = std::shared_timed_mutex;

    template <typename Mutex>
    using shared_lock = std::shared_lock<Mutex>;

    inline filesystem::path relative(
            const filesystem::path& path,
            const filesystem::path& base,
            std::error_code& ec) {
        ec.clear();

        try {
            const filesystem::path absolute_path = filesystem::absolute(path);
            const filesystem::path absolute_base = filesystem::absolute(base);

            auto path_it = absolute_path.begin();
            auto base_it = absolute_base.begin();

            while (path_it != absolute_path.end() && base_it != absolute_base.end() && *path_it == *base_it) {
                ++path_it;
                ++base_it;
            }

            filesystem::path result;
            for (; base_it != absolute_base.end(); ++base_it) {
                result /= "..";
            }
            for (; path_it != absolute_path.end(); ++path_it) {
                result /= *path_it;
            }

            if (result.empty()) {
                result = ".";
            }

            return result;
        } catch (const filesystem::filesystem_error& error) {
            ec = error.code();
            return filesystem::path();
        }
    }

#else

    namespace filesystem = std::experimental::filesystem;

    class shared_mutex {
    public:
        void lock() {
            m_mutex.lock();
        }

        void unlock() {
            m_mutex.unlock();
        }

        bool try_lock() {
            return m_mutex.try_lock();
        }

        void lock_shared() {
            m_mutex.lock();
        }

        void unlock_shared() {
            m_mutex.unlock();
        }

        bool try_lock_shared() {
            return m_mutex.try_lock();
        }

    private:
        std::mutex m_mutex;
    };

    template <typename Mutex>
    using shared_lock = std::unique_lock<Mutex>;

    inline filesystem::path relative(
            const filesystem::path& path,
            const filesystem::path& base,
            std::error_code& ec) {
        ec.clear();

        try {
            const filesystem::path absolute_path = filesystem::absolute(path);
            const filesystem::path absolute_base = filesystem::absolute(base);

            auto path_it = absolute_path.begin();
            auto base_it = absolute_base.begin();

            while (path_it != absolute_path.end() && base_it != absolute_base.end() && *path_it == *base_it) {
                ++path_it;
                ++base_it;
            }

            filesystem::path result;
            for (; base_it != absolute_base.end(); ++base_it) {
                result /= "..";
            }
            for (; path_it != absolute_path.end(); ++path_it) {
                result /= *path_it;
            }

            if (result.empty()) {
                result = ".";
            }

            return result;
        } catch (const filesystem::filesystem_error& error) {
            ec = error.code();
            return filesystem::path();
        }
    }

#endif

} // namespace compat
} // namespace consolix

#endif // _CONSOLIX_STD_COMPAT_HPP_INCLUDED
