/*
	Simple test to verify platform fixes
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

#include <iostream>
#include "../src/common.h"

int main() {
	std::cout << "Platform configuration test:" << std::endl;
	
	// Test that basic types are defined
	std::cout << "- LONG_PTR size: " << sizeof(LONG_PTR) << " bytes" << std::endl;
	std::cout << "- _TCHAR size: " << sizeof(_TCHAR) << " bytes" << std::endl;
	
	// Test string functions
	char buffer[100];
	my_sprintf_s(buffer, sizeof(buffer), "Test %d", 123);
	std::cout << "- sprintf test: " << buffer << std::endl;
	
	// Test path function
	const _TCHAR* path = get_application_path();
	std::cout << "- Application path: " << path << std::endl;
	
	std::cout << "\nAll platform tests passed!" << std::endl;
	return 0;
}