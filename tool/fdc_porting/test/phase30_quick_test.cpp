#include <iostream>
#include <cstring>
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

int main() {
    std::cout << "\n=== Phase 30: Type I Fix Test ===\n";
    
    // Initialize environment
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    // Create FDC
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Enable motor and select drive 0
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    fdc.write_signal(SIG_MB8877_DRIVEREG, 0, 3);
    event.advance_clock(1000);
    
    // Open disk
    fdc.open_disk(0, _T("test_disks/test_2d_patterns.d88"), 0);
    event.advance_clock(1000);
    
    std::cout << "\n1. Test READ without positioning (should fail):\n";
    fdc.write_io8(1, 0);    // Track register = 0
    fdc.write_io8(2, 1);    // Sector register = 1
    fdc.write_io8(0, 0x80); // READ SECTOR
    
    // Wait for completion
    for (int i = 0; i < 100 && (fdc.read_io8(0) & 0x01); i++) {
        event.advance_clock(10000);
    }
    
    uint32_t status = fdc.read_io8(0);
    std::cout << "   Status: 0x" << std::hex << status << std::dec;
    if (status & 0x10) {
        std::cout << " [RNF - Expected]\n";
    } else {
        std::cout << " [Unexpected success]\n";
    }
    
    std::cout << "\n2. RESTORE command (position to track 0):\n";
    fdc.write_io8(0, 0x08); // RESTORE with head load
    
    // Wait for completion
    for (int i = 0; i < 100 && (fdc.read_io8(0) & 0x01); i++) {
        event.advance_clock(10000);
    }
    
    status = fdc.read_io8(0);
    uint32_t trkreg = fdc.read_io8(1);
    std::cout << "   Status: 0x" << std::hex << status << std::dec;
    std::cout << ", Track Register: " << trkreg;
    std::cout << ", Track 00: " << ((status & 0x04) ? "YES" : "NO") << "\n";
    
    std::cout << "\n3. READ after RESTORE (should succeed):\n";
    fdc.write_io8(2, 1);    // Sector register = 1
    fdc.write_io8(0, 0x80); // READ SECTOR
    
    // Wait for completion
    for (int i = 0; i < 100 && (fdc.read_io8(0) & 0x01); i++) {
        event.advance_clock(10000);
    }
    
    status = fdc.read_io8(0);
    std::cout << "   Status: 0x" << std::hex << status << std::dec;
    if (status & 0x10) {
        std::cout << " [RNF - FAILED]\n";
    } else {
        std::cout << " [SUCCESS!]\n";
    }
    
    std::cout << "\n=== Summary ===\n";
    std::cout << "Fix applied: get_sector now uses correct track/side instead of -1/-1\n";
    std::cout << "This should eliminate 'Invalid track/side' errors\n";
    
    return 0;
}