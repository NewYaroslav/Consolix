#include <iostream>

namespace consolix {
class ConsoleApplication;
}

extern "C" consolix::ConsoleApplication* get_console_app_a();
extern "C" consolix::ConsoleApplication* get_console_app_b();
extern "C" void consolix_static_library_touch();

int main() {
	consolix_static_library_touch();
	
	auto* App_a = get_console_app_a();
	auto* App_b = get_console_app_b();
	
	std::cout 	<< "ConsoleApplication A address: " << static_cast<const void*>(App_a) 
				<< "	ConsoleApplication B address: " << static_cast<const void*>(App_b) 
				<< std::endl;
				
	if (App_a != App_b) {
		std::cout << "There are 2 different ConsoleApplication instances!" << std::endl;
		return 1;
	}
	
	std::cout << "There's only ConsoleApplication, singlton works correctly" << std::endl;
	return 0;
}
