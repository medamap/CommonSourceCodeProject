#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../../src/vm/mb8877_compat.h"
#include "../../../src/vm/mb8877_compat.cpp"
#include "../../../src/vm/disk.h"
#include "../../../src/vm/disk.cpp"
#include "../../../src/vm/noise.h"
#include "../../../src/vm/noise.cpp"

// Simple timer implementation
class SimpleTimer {
    uint64_t current_time;
public:
    SimpleTimer() : current_time(0) {}
    void advance(uint64_t usec) { current_time += usec; }
    uint64_t get_current_clock() { return current_time; }
};

SimpleTimer timer;

// Stub get_current_clock
uint64_t get_current_clock() {
    return timer.get_current_clock();
}

// Wait helper
void wait_for_completion(MB8877* fdc) {
    int timeout = 1000;
    while (timeout-- > 0 && (fdc->read_io8(0) & 0x01)) {
        timer.advance(1000);
        fdc->event_callback(0, 0);
        fdc->event_callback(1, 0);
        fdc->event_callback(2, 0);
        fdc->event_callback(3, 0);
    }
}

void test_type1_and_type2() {
    printf("\n=== Phase 30: Type I Command Implementation Test ===\n");
    
    MB8877 fdc(NULL, NULL);
    fdc.initialize();
    
    // Load test disk
    DISK* disk = new DISK(4);
    if (!disk->create_d88_image("test_disks/type2_test.d88")) {
        printf("ERROR: Failed to load test disk\n");
        delete disk;
        return;
    }
    
    // Connect disk
    fdc.set_drive_type(0, DRIVE_TYPE_2D);
    fdc.set_disk_handler(0, disk);
    
    // Motor on and select drive 0
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    fdc.write_signal(SIG_MB8877_DRIVEREG, 0, 3);
    
    printf("\nInitial state:\n");
    uint8_t status = fdc.read_io8(0);
    uint8_t trkreg = fdc.read_io8(1);
    printf("  Status: 0x%02X, Track Register: 0x%02X\n", status, trkreg);
    
    // Test 1: Direct READ without positioning
    printf("\n1. Direct READ without positioning (expect failure):\n");
    fdc.write_io8(1, 0);    // Track register = 0
    fdc.write_io8(2, 1);    // Sector register = 1
    fdc.write_io8(0, 0x80); // READ SECTOR command
    wait_for_completion(&fdc);
    status = fdc.read_io8(0);
    printf("  Status: 0x%02X %s\n", status, 
           (status & 0x10) ? "[RNF - Expected fail]" : "[Unexpected success]");
    
    // Test 2: RESTORE command
    printf("\n2. RESTORE command (position head to track 0):\n");
    fdc.write_io8(0, 0x08); // RESTORE with head load
    wait_for_completion(&fdc);
    status = fdc.read_io8(0);
    trkreg = fdc.read_io8(1);
    printf("  Status: 0x%02X, Track Register: 0x%02X\n", status, trkreg);
    printf("  Track 00 flag: %s\n", (status & 0x04) ? "SET [Good]" : "NOT SET [Bad]");
    
    // Test 3: READ after RESTORE
    printf("\n3. READ after RESTORE (should succeed):\n");
    fdc.write_io8(2, 1);    // Sector register = 1
    fdc.write_io8(0, 0x80); // READ SECTOR command
    wait_for_completion(&fdc);
    status = fdc.read_io8(0);
    printf("  Status: 0x%02X %s\n", status,
           (status & 0x10) ? "[RNF - Failed]" : "[Success!]");
    
    // Test 4: SEEK to track 10
    printf("\n4. SEEK to track 10:\n");
    fdc.write_io8(3, 10);   // Data register = 10
    fdc.write_io8(0, 0x18); // SEEK with head load
    wait_for_completion(&fdc);
    status = fdc.read_io8(0);
    trkreg = fdc.read_io8(1);
    printf("  Status: 0x%02X, Track Register: 0x%02X %s\n", status, trkreg,
           (trkreg == 10) ? "[Good]" : "[Bad]");
    
    // Test 5: READ at track 10
    printf("\n5. READ at track 10:\n");
    fdc.write_io8(2, 1);    // Sector register = 1
    fdc.write_io8(0, 0x80); // READ SECTOR command
    wait_for_completion(&fdc);
    status = fdc.read_io8(0);
    printf("  Status: 0x%02X %s\n", status,
           (status & 0x10) ? "[RNF - Failed]" : "[Success!]");
    
    // Test 6: STEP IN command
    printf("\n6. STEP IN (to track 11):\n");
    fdc.write_io8(0, 0x58); // STEP IN with head load and update
    wait_for_completion(&fdc);
    status = fdc.read_io8(0);
    trkreg = fdc.read_io8(1);
    printf("  Status: 0x%02X, Track Register: 0x%02X %s\n", status, trkreg,
           (trkreg == 11) ? "[Good]" : "[Bad]");
    
    // Test 7: STEP OUT command
    printf("\n7. STEP OUT (back to track 10):\n");
    fdc.write_io8(0, 0x78); // STEP OUT with head load and update
    wait_for_completion(&fdc);
    status = fdc.read_io8(0);
    trkreg = fdc.read_io8(1);
    printf("  Status: 0x%02X, Track Register: 0x%02X %s\n", status, trkreg,
           (trkreg == 10) ? "[Good]" : "[Bad]");
    
    // Summary
    printf("\n=== Summary ===\n");
    printf("Type I commands (RESTORE/SEEK/STEP) appear to be implemented.\n");
    printf("The issue is likely in how Type II commands use the positioning.\n");
    
    delete disk;
}

int main() {
    test_type1_and_type2();
    return 0;
}