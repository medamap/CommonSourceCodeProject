#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include "mb8877_test_helper.h"
#include "mb8877_test_wrapper.h"

// Enable debug logging in MB8877
#ifdef _FDC_DEBUG_LOG
#undef _FDC_DEBUG_LOG
#endif
#define _FDC_DEBUG_LOG 1

class ReadDebugTracer {
private:
    TestMB8877* fdc;
    MockDisk* disk;
    
    void trace_event(const char* event_name, int drvreg, int sector, int index, uint8_t status) {
        printf("[TRACE] %s: drvreg=%d, sector=%d, index=%d, status=0x%02X\n", 
               event_name, drvreg, sector, index, status);
    }
    
    void trace_buffer_state(const char* context, int drvreg) {
        printf("[BUFFER] %s: ", context);
        if (fdc && drvreg < 4) {
            // Access internal FDC state to check buffer
            printf("buffer_count=%d, cur_position=%d\n", 
                   fdc->get_buffer_count(drvreg), 
                   fdc->get_cur_position(drvreg));
        } else {
            printf("Invalid state\n");
        }
    }
    
public:
    ReadDebugTracer() : fdc(nullptr), disk(nullptr) {}
    
    void trace_read_sector_flow() {
        printf("\n=== READ SECTOR DEBUG TRACE ===\n");
        
        // Initialize FDC and disk
        fdc = new TestMB8877();
        fdc->initialize();
        fdc->reset();
        
        disk = new MockDisk();
        disk->setup_basic_disk();
        fdc->set_disk_handler(0, disk);
        
        // Motor on
        fdc->write_io8(0x08, 0x1C);
        
        // Setup track and sector
        fdc->write_io8(1, 0);  // Track 0
        fdc->write_io8(2, 1);  // Sector 1
        
        printf("\n[PHASE 1] Before READ SECTOR command\n");
        trace_buffer_state("Initial", 0);
        
        // Issue READ SECTOR command
        printf("\n[PHASE 2] Issuing READ SECTOR (0x80)\n");
        fdc->write_io8(0, 0x80);
        
        // Process events to find sector
        printf("\n[PHASE 3] Processing EVENT_SEARCH\n");
        for (int i = 0; i < 50 && (fdc->read_io8(0) & 0x01); i++) {
            fdc->event_callback(0, 0);  // Process events
            
            uint8_t status = fdc->read_io8(0);
            if (status & 0x02) {  // DRQ set
                printf("[DRQ SET] Status=0x%02X at iteration %d\n", status, i);
                trace_buffer_state("After DRQ", 0);
                break;
            }
        }
        
        // Try to read data
        printf("\n[PHASE 4] Reading sector data\n");
        uint8_t status = fdc->read_io8(0);
        printf("Status before read: 0x%02X (BUSY=%d, DRQ=%d)\n", 
               status, (status & 0x01) ? 1 : 0, (status & 0x02) ? 1 : 0);
        
        if (status & 0x02) {  // DRQ set
            printf("Reading first 16 bytes:\n");
            for (int i = 0; i < 16; i++) {
                uint8_t data = fdc->read_io8(3);
                printf("  [%02d] = 0x%02X", i, data);
                if (i < 256 && disk->sector) {
                    printf(" (expected: 0x%02X)", disk->sector[i]);
                }
                printf("\n");
                
                // Check buffer state periodically
                if (i % 4 == 3) {
                    trace_buffer_state("During read", 0);
                }
            }
        } else {
            printf("ERROR: DRQ not set, cannot read data\n");
        }
        
        // Cleanup
        delete fdc;
        delete disk;
    }
    
    void trace_event_search_behavior() {
        printf("\n=== EVENT_SEARCH BEHAVIOR TRACE ===\n");
        
        // This will require access to internal MB8877 event handling
        // We'll need to add debug hooks in MB8877 to trace this properly
        printf("TODO: Add event hooks in MB8877 for detailed tracing\n");
    }
};

int main() {
    ReadDebugTracer tracer;
    
    // Trace READ SECTOR flow
    tracer.trace_read_sector_flow();
    
    // Trace EVENT_SEARCH behavior
    tracer.trace_event_search_behavior();
    
    return 0;
}