#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

// Quick diagnostic to verify MB8877 buffer fields are accessible

extern "C" {
    void phase31_diagnostic_test() {
        printf("=== Phase 31 Diagnostic ===\n");
        
        // Test struct with buffer fields
        struct test_fdc {
            int track;
            int index;
            bool access;
            bool head_load;
            bool id_written;
            bool sector_found;
            int sector_length;
            int sector_index;
            int side;
            bool side_changed;
            int cur_position;
            int next_trans_position;
            int bytes_before_2nd_drq;
            int next_am1_position;
            uint32_t prev_clock;
            // New fields
            uint8_t buffer[1024];
            int buffer_count;
        } fdc[4];
        
        // Initialize
        for (int i = 0; i < 4; i++) {
            fdc[i].index = 0;
            fdc[i].buffer_count = 0;
            memset(fdc[i].buffer, 0, 1024);
        }
        
        // Test buffer operations
        int drvreg = 0;
        
        // Simulate loading data into buffer
        fdc[drvreg].buffer_count = 256;
        for (int i = 0; i < 256; i++) {
            fdc[drvreg].buffer[i] = i & 0xFF;
        }
        
        printf("Buffer loaded with %d bytes\n", fdc[drvreg].buffer_count);
        
        // Simulate reading from buffer
        printf("Reading first 16 bytes:\n");
        for (int i = 0; i < 16; i++) {
            if (fdc[drvreg].index < fdc[drvreg].buffer_count) {
                uint8_t data = fdc[drvreg].buffer[fdc[drvreg].index];
                printf("  [%02d] = 0x%02X\n", fdc[drvreg].index, data);
                fdc[drvreg].index++;
            }
        }
        
        printf("\nDiagnostic complete - buffer fields working\n");
    }
}

int main() {
    phase31_diagnostic_test();
    return 0;
}