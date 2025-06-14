/*
	Phase 29: Simple Type II Command Test
	Direct test to measure basic Type II functionality
*/

#include <iostream>
#include <cstring>
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

// MB8877 status register bits
#define STATUS_BUSY      0x01
#define STATUS_DRQ       0x02
#define STATUS_LOST_DATA 0x04
#define STATUS_RECORD_NOT_FOUND 0x10
#define STATUS_NOT_READY 0x80

int main() {
    std::cout << "Phase 29: Simple Type II Command Test\n";
    std::cout << "=====================================\n\n";
    
    // Initialize environment
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    SignalCapture drq_capture(&vm, &emu);
    
    // Create FDC
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.set_context_drq(&drq_capture, 0, 0xFFFFFFFF);
    fdc.initialize();
    fdc.reset();
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    event.advance_clock(1000);
    
    // Test 1: Check initial status
    std::cout << "Test 1: Initial status check\n";
    uint32_t status = fdc.read_io8(0);
    std::cout << "  Status register: 0x" << std::hex << status << std::dec << "\n";
    
    if (status & STATUS_NOT_READY) {
        std::cout << "  Disk NOT READY - this is expected without disk\n";
    }
    
    // Test 2: Try to open disk
    std::cout << "\nTest 2: Open disk image\n";
    fdc.open_disk(0, _T("test_disks/test_2d_patterns.d88"), 0);
    
    // Force interrupt to update status
    fdc.write_io8(0, 0xD0);
    event.advance_clock(1000);
    
    status = fdc.read_io8(0);
    std::cout << "  Status after disk open: 0x" << std::hex << status << std::dec << "\n";
    
    if (!(status & STATUS_NOT_READY)) {
        std::cout << "  Disk is READY\n";
        
        // Test 3: Read sector
        std::cout << "\nTest 3: Read sector command\n";
        
        // Set track 0, sector 1
        fdc.write_io8(1, 0); // Track register
        fdc.write_io8(2, 1); // Sector register
        
        std::cout << "  Issuing READ SECTOR command (0x80)\n";
        fdc.write_io8(0, 0x80); // Read sector command
        
        // Check immediate status
        status = fdc.read_io8(0);
        std::cout << "  Status after command: 0x" << std::hex << status << std::dec << "\n";
        
        if (status & STATUS_BUSY) {
            std::cout << "  BUSY flag set - command accepted\n";
            
            // Wait for DRQ
            int wait_cycles = 0;
            bool drq_found = false;
            
            while (wait_cycles < 100) {
                event.advance_clock(1000);
                status = fdc.read_io8(0);
                
                if (status & STATUS_DRQ) {
                    std::cout << "  DRQ set after " << wait_cycles << " cycles\n";
                    drq_found = true;
                    break;
                }
                
                if (!(status & STATUS_BUSY)) {
                    std::cout << "  Command completed without DRQ\n";
                    break;
                }
                
                wait_cycles++;
            }
            
            if (drq_found) {
                std::cout << "  Reading sector data...\n";
                
                // Read first few bytes
                for (int i = 0; i < 16; i++) {
                    uint8_t data = fdc.read_io8(3);
                    if (i == 0) std::cout << "  First 16 bytes: ";
                    std::cout << std::hex << (int)data << " ";
                }
                std::cout << std::dec << "\n";
                
                // Read rest of sector
                for (int i = 16; i < 256; i++) {
                    fdc.read_io8(3);
                }
                
                // Check final status
                status = fdc.read_io8(0);
                std::cout << "  Final status: 0x" << std::hex << status << std::dec << "\n";
                
                if (!(status & STATUS_BUSY) && !(status & STATUS_DRQ)) {
                    std::cout << "  READ SECTOR completed successfully!\n";
                    std::cout << "\n✓ Type II READ SECTOR command functional\n";
                    return 0;
                }
            }
        } else {
            std::cout << "  ERROR: BUSY flag not set - command rejected\n";
        }
    } else {
        std::cout << "  ERROR: Disk still NOT READY after open\n";
    }
    
    std::cout << "\n✗ Type II command test failed\n";
    return 1;
}