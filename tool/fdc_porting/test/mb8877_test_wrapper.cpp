/*
	MB8877 Test Wrapper
	
	This file provides the proper include order for testing MB8877 implementation
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

// Prevent including real headers that would conflict with mock environment
#define _EMU_H_
#define _DEVICE_H_
#define _VM_TEMPLATE_H_
#define _DISK_H_
#define _NOISE_H_
#define _FILEIO_H_

// Include mock environment first to provide all required types
#include "mock_environment.h"

// Include the header (not the .cpp) to get class definition
#include "../src/vm/mb8877_compat.h"

// Now include the implementation
#include "../src/vm/mb8877_compat.cpp"
