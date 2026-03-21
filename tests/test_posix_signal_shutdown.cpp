#include <atomic>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include <consolix/core.hpp>

namespace {

struct ChildStatus {
    int signal_value;
    int shutdown_calls;
    int resource_destroyed;
    int worker_started;
    int worker_joined;
    int resource_released;
};

class ResourceState {
public:
    explicit ResourceState(std::atomic<int>& destroyed_count) :
        m_destroyed_count(destroyed_count) {
    }

    ~ResourceState() {
        m_destroyed_count.fetch_add(1);
    }

private:
    std::atomic<int>& m_destroyed_count;
};

bool wait_until_readable(int fd, int timeout_ms) {
    struct pollfd descriptor;
    descriptor.fd = fd;
    descriptor.events = POLLIN;
    descriptor.revents = 0;

    const int result = ::poll(&descriptor, 1, timeout_ms);
    if (result < 0) {
        throw std::runtime_error("poll() failed");
    }
    if (result == 0) {
        return false;
    }
    return (descriptor.revents & POLLIN) != 0;
}

void write_all(int fd, const void* data, std::size_t size) {
    const char* cursor = static_cast<const char*>(data);
    std::size_t written = 0;

    while (written < size) {
        const ssize_t chunk = ::write(fd, cursor + written, size - written);
        if (chunk < 0) {
            throw std::runtime_error("write() failed");
        }
        written += static_cast<std::size_t>(chunk);
    }
}

bool read_all_with_timeout(int fd, void* data, std::size_t size, int timeout_ms) {
    char* cursor = static_cast<char*>(data);
    std::size_t received = 0;

    while (received < size) {
        if (!wait_until_readable(fd, timeout_ms)) {
            return false;
        }

        const ssize_t chunk = ::read(fd, cursor + received, size - received);
        if (chunk < 0) {
            throw std::runtime_error("read() failed");
        }
        if (chunk == 0) {
            return false;
        }
        received += static_cast<std::size_t>(chunk);
    }

    return true;
}

bool wait_for_exit(pid_t pid, int& status_code, int timeout_ms) {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);

    while (std::chrono::steady_clock::now() < deadline) {
        const pid_t result = ::waitpid(pid, &status_code, WNOHANG);
        if (result == pid) {
            return true;
        }
        if (result < 0) {
            throw std::runtime_error("waitpid() failed");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return false;
}

class SignalAwareComponent : public consolix::BaseLoopComponent {
public:
    SignalAwareComponent(int ready_fd, int status_fd) :
        m_ready_fd(ready_fd),
        m_status_fd(status_fd) {
    }

    virtual ~SignalAwareComponent() override {
        request_stop_and_join();
        close_fd(m_ready_fd);
        close_fd(m_status_fd);
    }

    bool on_once() override {
        m_resource.reset(new ResourceState(m_resource_destroyed));
        m_worker = std::thread(&SignalAwareComponent::worker_loop, this);

        const char ready = 'R';
        write_all(m_ready_fd, &ready, sizeof(ready));
        close_fd(m_ready_fd);
        return true;
    }

    void on_loop() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    void on_shutdown(int signal) override {
        m_shutdown_calls.fetch_add(1);
        request_stop_and_join();
        m_resource.reset();

        ChildStatus status;
        status.signal_value = signal;
        status.shutdown_calls = m_shutdown_calls.load();
        status.resource_destroyed = m_resource_destroyed.load();
        status.worker_started = m_worker_started.load();
        status.worker_joined = m_worker_joined.load();
        status.resource_released = m_resource ? 0 : 1;

        write_all(m_status_fd, &status, sizeof(status));
        close_fd(m_status_fd);
    }

private:
    void request_stop_and_join() {
        m_worker_stop.store(true);
        if (m_worker.joinable()) {
            m_worker.join();
            m_worker_joined.store(1);
        }
    }

    void worker_loop() {
        m_worker_started.store(1);
        while (!m_worker_stop.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    static void close_fd(int& fd) {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }

    int                   m_ready_fd;
    int                   m_status_fd;
    std::thread           m_worker;
    std::unique_ptr<ResourceState> m_resource;
    std::atomic<bool>     m_worker_stop{false};
    std::atomic<int>      m_shutdown_calls{0};
    std::atomic<int>      m_resource_destroyed{0};
    std::atomic<int>      m_worker_started{0};
    std::atomic<int>      m_worker_joined{0};
};

int run_child(int ready_fd, int status_fd) {
    try {
        consolix::add<SignalAwareComponent>(ready_fd, status_fd);
        consolix::run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Child error: " << e.what() << std::endl;
        return 1;
    }
}

void close_pipe_end(int& fd) {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

void fail_and_cleanup_child(pid_t pid, const std::string& message) {
    if (pid > 0) {
        ::kill(pid, SIGKILL);
        int status_code = 0;
        ::waitpid(pid, &status_code, 0);
    }
    throw std::runtime_error(message);
}

void validate_status(const ChildStatus& status, int expected_signal) {
    if (status.signal_value != expected_signal) {
        throw std::runtime_error("Unexpected shutdown signal value");
    }
    if (status.shutdown_calls != 1) {
        throw std::runtime_error("Shutdown callback must run exactly once");
    }
    if (status.worker_started != 1) {
        throw std::runtime_error("Worker thread did not start");
    }
    if (status.worker_joined != 1) {
        throw std::runtime_error("Worker thread was not joined");
    }
    if (status.resource_destroyed != 1) {
        throw std::runtime_error("Owned resource was not destroyed exactly once");
    }
    if (status.resource_released != 1) {
        throw std::runtime_error("Owned resource was not released before exit");
    }
}

void run_signal_scenario(int signal_value) {
    int ready_pipe[2] = {-1, -1};
    int status_pipe[2] = {-1, -1};

    if (::pipe(ready_pipe) != 0) {
        throw std::runtime_error("pipe() failed for ready_pipe");
    }
    if (::pipe(status_pipe) != 0) {
        close_pipe_end(ready_pipe[0]);
        close_pipe_end(ready_pipe[1]);
        throw std::runtime_error("pipe() failed for status_pipe");
    }

    const pid_t pid = ::fork();
    if (pid < 0) {
        close_pipe_end(ready_pipe[0]);
        close_pipe_end(ready_pipe[1]);
        close_pipe_end(status_pipe[0]);
        close_pipe_end(status_pipe[1]);
        throw std::runtime_error("fork() failed");
    }

    if (pid == 0) {
        close_pipe_end(ready_pipe[0]);
        close_pipe_end(status_pipe[0]);

        const int child_exit_code = run_child(ready_pipe[1], status_pipe[1]);
        close_pipe_end(ready_pipe[1]);
        close_pipe_end(status_pipe[1]);
        ::_exit(child_exit_code);
    }

    close_pipe_end(ready_pipe[1]);
    close_pipe_end(status_pipe[1]);

    try {
        char ready = 0;
        if (!read_all_with_timeout(ready_pipe[0], &ready, sizeof(ready), 3000) || ready != 'R') {
            fail_and_cleanup_child(pid, "Child did not report readiness");
        }

        if (::kill(pid, signal_value) != 0) {
            fail_and_cleanup_child(pid, "kill() failed");
        }

        ChildStatus status;
        std::memset(&status, 0, sizeof(status));
        if (!read_all_with_timeout(status_pipe[0], &status, sizeof(status), 3000)) {
            fail_and_cleanup_child(pid, "Did not receive shutdown status from child");
        }

        int wait_status = 0;
        if (!wait_for_exit(pid, wait_status, 5000)) {
            fail_and_cleanup_child(pid, "Child did not exit in time");
        }

        validate_status(status, signal_value);

        if (!WIFEXITED(wait_status)) {
            throw std::runtime_error("Child did not exit normally");
        }
        if (WEXITSTATUS(wait_status) != signal_value) {
            throw std::runtime_error("Unexpected child exit code");
        }
    } catch (...) {
        close_pipe_end(ready_pipe[0]);
        close_pipe_end(status_pipe[0]);
        throw;
    }

    close_pipe_end(ready_pipe[0]);
    close_pipe_end(status_pipe[0]);
}

} // namespace

int main() {
    try {
        run_signal_scenario(SIGINT);
        run_signal_scenario(SIGTERM);
        std::cout << "POSIX shutdown signal checks passed." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "POSIX signal shutdown test failed: " << e.what() << std::endl;
        return 1;
    }
}
