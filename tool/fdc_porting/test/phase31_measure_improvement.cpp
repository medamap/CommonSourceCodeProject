#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include "mb8877_test_helper.h"

// Phase 31: Measure READ operation improvements

class Phase31Tester : public TestBase {
public:
    bool test_read_basic() {
        printf("\n[TEST] Basic READ SECTOR\n");
        
        // Motor on
        fdc->write_io8(0x08, 0x1C);
        
        // Set track and sector
        fdc->write_io8(1, 0);  // Track 0
        fdc->write_io8(2, 1);  // Sector 1
        
        // Issue READ SECTOR command
        fdc->write_io8(0, 0x80);
        
        // Wait for DRQ
        if (!waitForDRQ()) {
            printf("  FAIL: DRQ not set\n");
            return false;
        }
        
        // Read 256 bytes
        bool success = true;
        for (int i = 0; i < 256; i++) {
            uint8_t data = fdc->read_io8(3);
            uint8_t expected = i & 0xFF;
            if (data != expected) {
                printf("  FAIL: byte[%d] = 0x%02X, expected 0x%02X\n", i, data, expected);
                success = false;
                break;
            }
        }
        
        if (success) {
            printf("  PASS: All 256 bytes read correctly\n");
        }
        
        return success;
    }
    
    bool test_read_drq_timing() {
        printf("\n[TEST] READ DRQ Timing\n");
        
        // Motor on
        fdc->write_io8(0x08, 0x1C);
        
        // Set track and sector
        fdc->write_io8(1, 0);
        fdc->write_io8(2, 2);
        
        // Issue READ SECTOR
        fdc->write_io8(0, 0x80);
        
        // Measure time to DRQ
        int cycles = 0;
        while (!(fdc->read_io8(0) & 0x02) && cycles < 10000) {
            fdc->event_callback(0, 0);
            cycles++;
        }
        
        if (cycles >= 10000) {
            printf("  FAIL: DRQ timeout after %d cycles\n", cycles);
            return false;
        }
        
        printf("  PASS: DRQ set after %d cycles\n", cycles);
        return true;
    }
    
    bool test_multi_sector_read() {
        printf("\n[TEST] Multi-sector READ\n");
        
        // Motor on
        fdc->write_io8(0x08, 0x1C);
        
        // Set track and start sector
        fdc->write_io8(1, 0);
        fdc->write_io8(2, 1);  // Start at sector 1
        
        // Issue MULTI-SECTOR READ (0x90)
        fdc->write_io8(0, 0x90);
        
        bool success = true;
        
        // Read 3 sectors
        for (int sector = 1; sector <= 3; sector++) {
            printf("  Reading sector %d:\n", sector);
            
            // Wait for DRQ
            if (!waitForDRQ()) {
                printf("    FAIL: DRQ not set for sector %d\n", sector);
                return false;
            }
            
            // Read sector data
            for (int i = 0; i < 256; i++) {
                uint8_t data = fdc->read_io8(3);
                uint8_t expected = ((sector - 1) * 100 + i) & 0xFF;
                if (data != expected) {
                    printf("    FAIL: sector %d, byte[%d] = 0x%02X, expected 0x%02X\n", 
                           sector, i, data, expected);
                    success = false;
                    break;
                }
            }
            
            if (!success) break;
            
            // Process events for next sector
            for (int i = 0; i < 100; i++) {
                fdc->event_callback(0, 0);
            }
        }
        
        if (success) {
            printf("  PASS: All 3 sectors read correctly\n");
        }
        
        return success;
    }
    
    bool test_read_io8_sequence() {
        printf("\n[TEST] read_io8 Sequence\n");
        
        // Motor on
        fdc->write_io8(0x08, 0x1C);
        
        // Set track and sector
        fdc->write_io8(1, 0);
        fdc->write_io8(2, 1);
        
        // Issue READ SECTOR
        fdc->write_io8(0, 0x80);
        
        // Wait for DRQ
        if (!waitForDRQ()) {
            printf("  FAIL: DRQ not set\n");
            return false;
        }
        
        // Test sequential reads
        printf("  Testing sequential reads:\n");
        bool success = true;
        for (int i = 0; i < 16; i++) {
            uint8_t data = fdc->read_io8(3);
            printf("    [%02d] = 0x%02X", i, data);
            if (data != (i & 0xFF)) {
                printf(" (FAIL: expected 0x%02X)", i & 0xFF);
                success = false;
            }
            printf("\n");
        }
        
        return success;
    }
};

void run_comprehensive_tests() {
    printf("\n=== Phase 31: READ Operation Improvement Tests ===\n");
    
    Phase31Tester tester;
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Basic READ
    total++;
    if (tester.test_read_basic()) passed++;
    
    // Test 2: DRQ timing
    total++;
    if (tester.test_read_drq_timing()) passed++;
    
    // Test 3: Multi-sector
    total++;
    if (tester.test_multi_sector_read()) passed++;
    
    // Test 4: read_io8 sequence
    total++;
    if (tester.test_read_io8_sequence()) passed++;
    
    // Results
    printf("\n=== RESULTS ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Success rate: %.1f%%\n", (passed * 100.0) / total);
    
    if (passed == total) {
        printf("\nSUCCESS: All READ operations fixed!\n");
    } else {
        printf("\nWARNING: Some READ operations still failing\n");
    }
}

int main() {
    run_comprehensive_tests();
    return 0;
}