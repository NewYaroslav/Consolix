#include "../../include/consolix/core/ConsoleApplication.hpp"
#include <memory>

extern "C" consolix::ConsoleApplication* get_console_app_b() { 
	return std::addressof(consolix::ConsoleApplication::get_instance()); 
}