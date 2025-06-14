/*
	MB8877 Test Wrapper with D88 Support
	
	Author : Claude AI Assistant
	Date   : 2025.06.13
*/

#ifndef _MB8877_TEST_D88_H_
#define _MB8877_TEST_D88_H_

#include "../../../src/vm/mb8877_compat.h"
#include "mock_disk_d88.h"

// Test-friendly MB8877 class that allows D88 disk injection
class MB8877_Test : public MB8877 {
public:
	MB8877_Test(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MB8877(parent_vm, parent_emu) {}
	
	// Override open_disk to use our D88-enabled disk
	void open_disk(int drv, const _TCHAR* file_path, int bank) {
		if (is_drive_valid(drv)) {
			// Close existing disk if any
			if (disk[drv]) {
				disk[drv]->close();
				delete disk[drv];
			}
			
			// Create our D88-enabled disk
			MockDISK_D88* d88_disk = new MockDISK_D88(vm, emu);
			d88_disk->open(file_path, bank);
			disk[drv] = d88_disk;
			
			// Update FDC state
			fdc[drv].access = true;
		}
	}
};

#endif // _MB8877_TEST_D88_H_