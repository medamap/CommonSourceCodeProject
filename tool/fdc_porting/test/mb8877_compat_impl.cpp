/*
	MB8877 Compatibility Implementation
	
	This file provides the MB8877 implementation for tests
	
	Author : Claude AI Assistant
	Date   : 2025.01.11
*/

// Use the compatibility version with safety features
#define USE_MB8877_COMPAT

// Prevent including real headers that would conflict with mock environment
#define _EMU_H_
#define _DEVICE_H_
#define _VM_TEMPLATE_H_
#define _DISK_H_
#define _NOISE_H_
#define _FILEIO_H_

// Include mock environment first to provide all required types
#include "mock_environment.h"

// Check if we're using the compat version which has get_intr_ack
#ifdef USE_MB8877_COMPAT
// Include the compatibility version
#include "../../../src/vm/mb8877_compat.h"
#include "../../../src/vm/mb8877_compat.cpp"
#else
// Include the original version
#include "../../../src/vm/mb8877.h"
#include "../../../src/vm/mb8877.cpp"
#endif