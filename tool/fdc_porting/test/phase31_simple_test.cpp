#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include "mb8877_test_wrapper.h"
#include "mock_environment.h"
#include "mock_disk_d88.h"

// Phase 31: Simple test to verify READ operation fixes

void test_basic_read() {
    printf("\n=== Phase 31: Basic READ Test ===\n");
    
    // Create test environment
    MockEnvironment env;
    MB8877TestWrapper wrapper(&env);
    MB8877* fdc = wrapper.getFDC();
    
    // Create and insert test disk
    MockDiskD88 disk;
    disk.setup_test_disk();
    fdc->set_disk_handler(0, &disk);
    
    // Initialize FDC
    fdc->initialize();
    fdc->reset();
    
    // Motor on
    fdc->write_io8(0x08, 0x1C);
    printf("Motor ON\n");
    
    // Set track and sector
    fdc->write_io8(1, 0);  // Track 0
    fdc->write_io8(2, 1);  // Sector 1
    printf("Set Track=0, Sector=1\n");
    
    // Issue READ SECTOR command
    printf("Issuing READ SECTOR (0x80)\n");
    fdc->write_io8(0, 0x80);
    
    // Process events to find sector
    printf("Processing events...\n");
    int cycles = 0;
    while ((fdc->read_io8(0) & 0x01) && cycles < 10000) {
        fdc->event_callback(0, 0);
        cycles++;
        
        // Check for DRQ
        if (fdc->read_io8(0) & 0x02) {
            printf("DRQ set after %d cycles\n", cycles);
            break;
        }
    }
    
    // Check status
    uint8_t status = fdc->read_io8(0);
    printf("Status: 0x%02X (BUSY=%d, DRQ=%d)\n", 
           status, (status & 0x01) ? 1 : 0, (status & 0x02) ? 1 : 0);
    
    if (!(status & 0x02)) {
        printf("FAIL: DRQ not set!\n");
        return;
    }
    
    // Read first 16 bytes
    printf("\nReading first 16 bytes:\n");
    bool success = true;
    for (int i = 0; i < 16; i++) {
        uint8_t data = fdc->read_io8(3);
        uint8_t expected = i & 0xFF;
        printf("  [%02d] = 0x%02X", i, data);
        if (data != expected) {
            printf(" (FAIL: expected 0x%02X)", expected);
            success = false;
        }
        printf("\n");
    }
    
    if (success) {
        printf("\nSUCCESS: READ operation working correctly!\n");
    } else {
        printf("\nFAIL: READ operation still has issues\n");
    }
}

int main() {
    test_basic_read();
    return 0;
}