#include <consolix/core.hpp>
#include <memory>

extern "C" void consolix_static_library_touch() {
    (void)std::addressof(consolix::ConsoleApplication::get_instance());
    (void)std::addressof(consolix::ServiceLocator::get_instance());
}