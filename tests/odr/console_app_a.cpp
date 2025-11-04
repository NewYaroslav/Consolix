#include "../../include/consolix/core/ConsoleApplication.hpp"

extern "C" consolix::ConsoleApplication* get_console_app_a() { 
	return std::addressof(consolix::ConsoleApplication::get_instance()); 
}