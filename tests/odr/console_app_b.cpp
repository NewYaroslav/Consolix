#include "../../include/consolix/core/ConsoleApplication.hpp"

extern "C" consolix::ConsoleApplication* get_console_app_b() { 
	return std::addressof(consolix::ConsoleApplication::get_instance()); 
}