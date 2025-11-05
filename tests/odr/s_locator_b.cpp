#include <consolix/core.hpp>
#include <memory>

extern "C" consolix::ServiceLocator* get_s_locator_b() { 
	return std::addressof(consolix::ServiceLocator::get_instance()); 
}