#include <iostream>
#include <cstdio>
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

int main() {
    std::cout << "Phase 40: Simple Disk Test\n";
    std::cout << "=========================\n";
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    
    std::cout << "Initializing FDC...\n";
    fdc.initialize();
    fdc.reset();
    
    // Check if disk file exists
    const char* disk_path = "data/test_disk_images/test_basic_2d.d88";
    FILE* check_file = fopen(disk_path, "rb");
    if(!check_file) {
        std::cout << "Error: Disk file not found: " << disk_path << "\n";
        return 1;
    }
    fclose(check_file);
    
    std::cout << "Opening disk: " << disk_path << "\n";
    fdc.open_disk(0, disk_path, 0);
    
    std::cout << "Turning on motor...\n";
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Try a simple restore command
    std::cout << "Issuing RESTORE command...\n";
    fdc.write_io8(0, 0x00);  // RESTORE command
    
    // Wait for completion
    int timeout = 100;
    while((fdc.read_io8(0) & 0x01) && timeout-- > 0) {
        event.advance_clock(1000);
    }
    
    uint32_t status = fdc.read_io8(0);
    std::cout << "Final status: 0x" << std::hex << status << std::dec << "\n";
    
    if(timeout > 0) {
        std::cout << "✓ Command completed successfully\n";
        std::cout << "  BUSY cleared: " << ((status & 0x01) ? "NO" : "YES") << "\n";
        std::cout << "  TRACK00: " << ((status & 0x04) ? "YES" : "NO") << "\n";
        std::cout << "  RNF: " << ((status & 0x10) ? "YES" : "NO") << "\n";
        return 0;
    } else {
        std::cout << "✗ Command timeout\n";
        return 1;
    }
}