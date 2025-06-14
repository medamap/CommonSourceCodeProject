// Phase 39: Simple test to verify Type I/II fixes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/vm/mb8877_compat.h"
#include "mock_environment.h"
#include "d88_loader.h"

int main() {
    printf("=== Phase 39 Simple Test ===\n");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Open a test disk
    fdc.open_disk(0, _T("test_disks/test.d88"), 0);
    
    // Enable motor
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    printf("\n--- Test 1: RESTORE command ---\n");
    // Issue RESTORE command
    fdc.write_io8(0, 0x00);  // RESTORE without verify
    event.advance_clock(100000);
    uint32_t status = fdc.read_io8(0);
    printf("After RESTORE: status=0x%02X (BUSY=%d, TR00=%d)\n", 
           status, (status & 0x01) != 0, (status & 0x04) != 0);
    uint32_t track = fdc.read_io8(1);
    printf("Track register = %d\n", track);
    
    printf("\n--- Test 2: SEEK command ---\n");
    // SEEK to track 10
    fdc.write_io8(3, 10);  // Data register = 10
    fdc.write_io8(0, 0x10);  // SEEK without verify
    event.advance_clock(100000);
    status = fdc.read_io8(0);
    printf("After SEEK: status=0x%02X (BUSY=%d)\n", status, (status & 0x01) != 0);
    track = fdc.read_io8(1);
    printf("Track register = %d (expected 10)\n", track);
    
    printf("\n--- Test 3: READ SECTOR ---\n");
    // Position to track 0, sector 1
    fdc.write_io8(1, 0);  // Track register
    fdc.write_io8(2, 1);  // Sector register
    fdc.write_io8(0, 0x80);  // READ SECTOR
    
    // Wait for search
    event.advance_clock(50000);
    status = fdc.read_io8(0);
    printf("During READ: status=0x%02X (BUSY=%d, DRQ=%d)\n", 
           status, (status & 0x01) != 0, (status & 0x02) != 0);
    
    // Read some data if DRQ is set
    if (status & 0x02) {
        uint8_t data = fdc.read_io8(3);
        printf("Read first byte: 0x%02X\n", data);
    }
    
    event.advance_clock(50000);
    status = fdc.read_io8(0);
    printf("After READ: status=0x%02X (BUSY=%d)\n", status, (status & 0x01) != 0);
    
    printf("\n--- Test 4: WRITE SECTOR (write protected) ---\n");
    // Try to write to protected disk
    fdc.write_io8(1, 0);  // Track register
    fdc.write_io8(2, 1);  // Sector register
    fdc.write_io8(0, 0xA0);  // WRITE SECTOR
    
    event.advance_clock(50000);
    status = fdc.read_io8(0);
    printf("After WRITE: status=0x%02X (BUSY=%d, WP=%d)\n", 
           status, (status & 0x01) != 0, (status & 0x40) != 0);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}