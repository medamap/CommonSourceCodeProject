// Auto-generated test wrapper for compat
#define STANDALONE_TEST

// Prevent including real headers that would conflict with mock environment
#define _EMU_H_
#define _DEVICE_H_
#define _VM_TEMPLATE_H_
#define _DISK_H_
#define _NOISE_H_
#define _FILEIO_H_

// Include mock environment first to provide all required types
#include "mock_environment.h"

// Include appropriate header based on implementation
#include "../../../src/vm/mb8877_compat.h"
#include "../../../src/vm/mb8877_compat.cpp"
