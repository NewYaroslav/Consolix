#include <iostream>
#include <consolix/core.hpp>

extern "C" consolix::ServiceLocator* get_s_locator_a();
extern "C" consolix::ServiceLocator* get_s_locator_b();

int main() {
	auto* Locator_a = get_s_locator_a();
	auto* Locator_b = get_s_locator_b();
	
	std::cout 	<< "ServiceLocator A address: " << static_cast<const void*>(Locator_a) 
				<< "	ServiceLocator B address: " << static_cast<const void*>(Locator_b) 
				<< std::endl;
				
	if (Locator_a != Locator_b) {
		std::cout << "There are 2 different ServiceLocator instances!" << std::endl;
		return 1;
	}
	
	std::cout << "There's only ServiceLocator, singlton works correctly" << std::endl;
	return 0;
}