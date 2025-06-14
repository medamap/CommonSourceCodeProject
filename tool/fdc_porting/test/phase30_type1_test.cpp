#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mb8877_test_helper.h"
#include "d88_loader.h"

void test_type1_commands() {
    printf("\n=== Phase 30: Type I Command Test ===\n");
    
    MB8877* fdc = create_mb8877();
    if (!fdc) {
        printf("ERROR: Failed to create FDC\n");
        return;
    }
    
    // Load test disk
    DISK* disk = new DISK(4);
    if (!disk->load_d88("test_disks/type2_test.d88")) {
        printf("ERROR: Failed to load test disk\n");
        delete disk;
        destroy_mb8877(fdc);
        return;
    }
    
    // Connect disk to drive 0
    fdc->set_drive_type(0, DRIVE_TYPE_2D);
    fdc->set_disk_handler(0, disk);
    
    // Motor on and select drive 0
    fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
    fdc->write_signal(SIG_MB8877_DRIVEREG, 0, 3);
    
    // Show initial state
    printf("\nInitial state:\n");
    uint8_t status = fdc->read_io8(0);
    uint8_t trkreg = fdc->read_io8(1);
    printf("  Status: 0x%02X, Track Register: 0x%02X\n", status, trkreg);
    
    // Test 1: Direct READ without positioning (should fail)
    printf("\n1. Direct READ without positioning:\n");
    fdc->write_io8(1, 0);    // Track register = 0
    fdc->write_io8(2, 1);    // Sector register = 1
    fdc->write_io8(0, 0x80); // READ SECTOR command
    wait_for_completion(fdc);
    status = fdc->read_io8(0);
    printf("  Status after READ: 0x%02X %s\n", status, 
           (status & 0x10) ? "[RNF - Expected]" : "[Unexpected]");
    
    // Test 2: RESTORE command
    printf("\n2. RESTORE command:\n");
    fdc->write_io8(0, 0x08); // RESTORE with head load
    wait_for_completion(fdc);
    status = fdc->read_io8(0);
    trkreg = fdc->read_io8(1);
    printf("  Status: 0x%02X, Track Register: 0x%02X\n", status, trkreg);
    printf("  Track 00 flag: %s\n", (status & 0x04) ? "SET" : "NOT SET");
    
    // Test 3: READ after RESTORE (should work)
    printf("\n3. READ after RESTORE:\n");
    fdc->write_io8(2, 1);    // Sector register = 1
    fdc->write_io8(0, 0x80); // READ SECTOR command
    wait_for_completion(fdc);
    status = fdc->read_io8(0);
    printf("  Status after READ: 0x%02X %s\n", status,
           (status & 0x10) ? "[RNF - Failed]" : "[Success]");
    
    // Test 4: SEEK to track 10
    printf("\n4. SEEK to track 10:\n");
    fdc->write_io8(3, 10);   // Data register = 10 (target track)
    fdc->write_io8(0, 0x18); // SEEK with head load
    wait_for_completion(fdc);
    status = fdc->read_io8(0);
    trkreg = fdc->read_io8(1);
    printf("  Status: 0x%02X, Track Register: 0x%02X\n", status, trkreg);
    
    // Test 5: READ at track 10
    printf("\n5. READ at track 10:\n");
    fdc->write_io8(2, 1);    // Sector register = 1
    fdc->write_io8(0, 0x80); // READ SECTOR command
    wait_for_completion(fdc);
    status = fdc->read_io8(0);
    printf("  Status after READ: 0x%02X %s\n", status,
           (status & 0x10) ? "[RNF - Failed]" : "[Success]");
    
    // Cleanup
    delete disk;
    destroy_mb8877(fdc);
}

int main(int argc, char* argv[]) {
    test_type1_commands();
    return 0;
}