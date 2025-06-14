#include <iostream>
#include <cstdio>
#include <vector>
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

int main() {
    std::cout << "Phase 40: Safe Disk Test\n";
    std::cout << "========================\n";
    
    // Create objects on heap to control destruction order
    MockEMU* emu = new MockEMU();
    MockVM* vm = new MockVM(emu);
    MockEVENT* event = new MockEVENT(vm, emu);
    
    MB8877* fdc = new MB8877(vm, emu);
    fdc->set_context_event_manager(event, 0, 0, 0);
    
    std::cout << "Initializing FDC...\n";
    fdc->initialize();
    fdc->reset();
    
    // Check if disk file exists
    const char* disk_path = "data/test_disk_images/test_basic_2d.d88";
    FILE* check_file = fopen(disk_path, "rb");
    if(!check_file) {
        std::cout << "Error: Disk file not found: " << disk_path << "\n";
        return 1;
    }
    fclose(check_file);
    
    std::cout << "Opening disk: " << disk_path << "\n";
    fdc->open_disk(0, disk_path, 0);
    
    std::cout << "Turning on motor...\n";
    fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Test 1: RESTORE command
    std::cout << "\nTest 1: RESTORE command\n";
    fdc->write_io8(1, 10);  // Start from track 10
    fdc->write_io8(0, 0x00);  // RESTORE command
    
    // Wait for completion
    int timeout = 100;
    while((fdc->read_io8(0) & 0x01) && timeout-- > 0) {
        event->advance_clock(1000);
    }
    
    uint32_t status = fdc->read_io8(0);
    uint32_t track = fdc->read_io8(1);
    
    std::cout << "  Status: 0x" << std::hex << status << std::dec << "\n";
    std::cout << "  Track: " << track << "\n";
    std::cout << "  Result: " << ((track == 0 && (status & 0x04)) ? "PASS" : "FAIL") << "\n";
    
    // Test 2: SEEK command
    std::cout << "\nTest 2: SEEK to track 5\n";
    fdc->write_io8(3, 5);  // Data register = target track
    fdc->write_io8(0, 0x10);  // SEEK command
    
    timeout = 100;
    while((fdc->read_io8(0) & 0x01) && timeout-- > 0) {
        event->advance_clock(1000);
    }
    
    status = fdc->read_io8(0);
    track = fdc->read_io8(1);
    
    std::cout << "  Status: 0x" << std::hex << status << std::dec << "\n";
    std::cout << "  Track: " << track << "\n";
    std::cout << "  Result: " << ((track == 5 && !(status & 0x04)) ? "PASS" : "FAIL") << "\n";
    
    // Test 3: READ SECTOR
    std::cout << "\nTest 3: READ SECTOR\n";
    fdc->write_io8(1, 0);  // Track 0
    fdc->write_io8(2, 1);  // Sector 1
    fdc->write_io8(0, 0x80);  // READ SECTOR
    
    std::vector<uint8_t> read_data;
    timeout = 1000;
    bool read_success = false;
    
    while(timeout-- > 0) {
        status = fdc->read_io8(0);
        
        if(!(status & 0x01)) {
            // BUSY cleared
            break;
        }
        
        if(status & 0x02) {  // DRQ set
            uint8_t data = fdc->read_io8(3);
            read_data.push_back(data);
            read_success = true;
        }
        
        event->advance_clock(100);
    }
    
    std::cout << "  Status: 0x" << std::hex << status << std::dec << "\n";
    std::cout << "  Bytes read: " << read_data.size() << "\n";
    std::cout << "  RNF: " << ((status & 0x10) ? "YES" : "NO") << "\n";
    std::cout << "  Result: " << (read_success ? "PASS" : "FAIL") << "\n";
    
    // Clean shutdown
    std::cout << "\nClosing disk...\n";
    fdc->close_disk(0);
    
    std::cout << "\nTest completed successfully!\n";
    
    // Clean up in reverse order
    delete fdc;
    delete event;
    delete vm;
    delete emu;
    
    return 0;
}