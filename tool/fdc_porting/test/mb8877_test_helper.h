#ifndef MB8877_TEST_HELPER_H
#define MB8877_TEST_HELPER_H

#include "../../../src/vm/mb8877_compat.h"
#include <cstdio>

class MB8877TestHelper {
private:
    MB8877* fdc;
    
public:
    MB8877TestHelper(MB8877* fdc_instance) : fdc(fdc_instance) {}
    
    // Internal state access methods
    uint8_t getStatus() const { return fdc->status; }
    uint8_t getCmdReg() const { return fdc->cmdreg; }
    uint8_t getTrackReg() const { return fdc->trkreg; }
    uint8_t getSectorReg() const { return fdc->secreg; }
    uint8_t getDataReg() const { return fdc->datareg; }
    uint8_t getCmdType() const { return fdc->cmdtype; }
    
    // Timing and event state
    int getRegisterID(int event_id) const {
        for(int i = 0; i < 8; i++) { // 8 event types
            if(fdc->register_id[i] == event_id) {
                return i;
            }
        }
        return -1;
    }
    
    bool isEventRegistered(int event_type) const {
        return fdc->register_id[event_type] != -1;
    }
    
    double getEventTime(int event_type) const {
        if(fdc->register_id[event_type] != -1) {
            // Note: We can't access EVENT directly, but we know the event is registered
            return -1.0; // Placeholder - would need EVENT access
        }
        return -1.0;
    }
    
    // Disk state
    int getCurrentDrive() const { return fdc->drvreg; }
    bool isDiskInserted(int drv = -1) const {
        if(drv == -1) drv = fdc->drvreg;
        return fdc->disk[drv] != nullptr && fdc->disk[drv]->inserted;
    }
    
    int getCurrentTrack(int drv = -1) const {
        if(drv == -1) drv = fdc->drvreg;
        return fdc->fdc[drv].track;
    }
    
    bool isMotorOn(int drv = -1) const {
        if(drv == -1) drv = fdc->drvreg;
        return fdc->motor_on; // Global motor state
    }
    
    bool isHeadLoaded(int drv = -1) const {
        if(drv == -1) drv = fdc->drvreg;
        return fdc->fdc[drv].head_load;
    }
    
    // Data transfer state
    int getDataCount() const { 
        // Count is stored in sector_length for current implementation
        return fdc->fdc[fdc->drvreg].sector_length; 
    }
    int getDataIndex() const { return fdc->fdc[fdc->drvreg].index; }
    uint8_t* getBuffer() const { 
        // Buffer data is accessed through disk->sector
        DISK* disk = fdc->disk[fdc->drvreg];
        return disk ? disk->sector : nullptr;
    }
    
    // Debug output methods
    void dumpState(const char* label = nullptr) const {
        if(label) {
            printf("\n=== MB8877 State: %s ===\n", label);
        } else {
            printf("\n=== MB8877 State ===\n");
        }
        
        printf("Registers:\n");
        printf("  Command:  0x%02X (Type %d)\n", getCmdReg(), getCmdType());
        printf("  Status:   0x%02X", getStatus());
        if(getStatus() & 0x80) printf(" NOTREADY");
        if(getStatus() & 0x40) printf(" WRITEPROT");
        if(getStatus() & 0x20) printf(" HEADLOAD");
        if(getStatus() & 0x10) printf(" SEEKERR");
        if(getStatus() & 0x08) printf(" CRCERR");
        if(getStatus() & 0x04) printf(" TRACK0");
        if(getStatus() & 0x02) printf(" DRQ");
        if(getStatus() & 0x01) printf(" BUSY");
        printf("\n");
        printf("  Track:    0x%02X (%d)\n", getTrackReg(), getTrackReg());
        printf("  Sector:   0x%02X (%d)\n", getSectorReg(), getSectorReg());
        printf("  Data:     0x%02X\n", getDataReg());
        
        printf("\nDrive %d state:\n", getCurrentDrive());
        printf("  Disk inserted: %s\n", isDiskInserted() ? "Yes" : "No");
        printf("  Track:         %d\n", getCurrentTrack());
        printf("  Motor:         %s\n", isMotorOn() ? "ON" : "OFF");
        printf("  Head loaded:   %s\n", isHeadLoaded() ? "Yes" : "No");
        
        printf("\nData transfer:\n");
        printf("  Count: %d\n", getDataCount());
        printf("  Index: %d\n", getDataIndex());
        
        printf("\nEvents:\n");
        for(int i = 0; i < 8; i++) {
            if(isEventRegistered(i)) {
                const char* event_names[] = {
                    "SEEK", "SEARCH", "DRQ", "MULTI",
                    "LOST", "5", "6", "7"
                };
                printf("  %s: Registered\n", event_names[i]);
            }
        }
        
        printf("==================\n\n");
    }
    
    void dumpBuffer(int max_bytes = 32) const {
        uint8_t* buffer = getBuffer();
        int count = getDataCount();
        int index = getDataIndex();
        
        printf("Buffer dump (count=%d, index=%d):\n", count, index);
        
        int bytes_to_show = (max_bytes > 0 && count > max_bytes) ? max_bytes : count;
        
        for(int i = 0; i < bytes_to_show; i++) {
            if(i % 16 == 0) {
                printf("  %04X: ", i);
            }
            
            if(i == index) {
                printf("[%02X]", buffer[i]);
            } else {
                printf(" %02X ", buffer[i]);
            }
            
            if(i % 16 == 15 || i == bytes_to_show - 1) {
                printf("\n");
            }
        }
        
        if(count > bytes_to_show) {
            printf("  ... (%d more bytes)\n", count - bytes_to_show);
        }
    }
    
    // Test support methods
    void waitForNotBusy(int max_cycles = 1000000) {
        int cycles = 0;
        while((getStatus() & 1) && cycles < max_cycles) {
            fdc->event_callback(0, 0); // Process events
            cycles++;
        }
        if(cycles >= max_cycles) {
            printf("WARNING: Timeout waiting for NOT BUSY after %d cycles\n", cycles);
        }
    }
    
    bool waitForDRQ(int max_cycles = 100000) {
        int cycles = 0;
        while(!(getStatus() & 2) && (getStatus() & 1) && cycles < max_cycles) {
            fdc->event_callback(0, 0); // Process events
            cycles++;
        }
        
        if(cycles >= max_cycles) {
            printf("WARNING: Timeout waiting for DRQ after %d cycles\n", cycles);
            return false;
        }
        
        return (getStatus() & 2) != 0;
    }
    
    // Verify sector data pattern
    bool verifySectorData(int track, int head, int sector, int expected_pattern) {
        uint8_t* buffer = getBuffer();
        int count = getDataCount();
        
        if(count != 256) {
            printf("ERROR: Expected 256 bytes, got %d\n", count);
            return false;
        }
        
        bool success = true;
        
        switch(expected_pattern) {
            case 0: // Sequential pattern
                for(int i = 0; i < 256; i++) {
                    uint8_t expected = (track * 100 + head * 10 + sector + i) % 256;
                    if(buffer[i] != expected) {
                        printf("ERROR: Byte %d: expected 0x%02X, got 0x%02X\n", 
                               i, expected, buffer[i]);
                        success = false;
                        if(i > 10) break; // Don't spam too many errors
                    }
                }
                break;
                
            case 1: // Sector ID pattern
                for(int i = 0; i < 256; i++) {
                    if(buffer[i] != sector) {
                        printf("ERROR: Byte %d: expected 0x%02X, got 0x%02X\n", 
                               i, sector, buffer[i]);
                        success = false;
                        if(i > 10) break;
                    }
                }
                break;
                
            case 2: // Alternating pattern
                for(int i = 0; i < 256; i++) {
                    uint8_t expected = (i % 2 == 0) ? 0x55 : 0xAA;
                    if(buffer[i] != expected) {
                        printf("ERROR: Byte %d: expected 0x%02X, got 0x%02X\n", 
                               i, expected, buffer[i]);
                        success = false;
                        if(i > 10) break;
                    }
                }
                break;
                
            case 3: // Track-based pattern
                for(int i = 0; i < 256; i++) {
                    if(buffer[i] != track) {
                        printf("ERROR: Byte %d: expected 0x%02X, got 0x%02X\n", 
                               i, track, buffer[i]);
                        success = false;
                        if(i > 10) break;
                    }
                }
                break;
                
            default:
                printf("ERROR: Unknown pattern type %d\n", expected_pattern);
                return false;
        }
        
        return success;
    }
};

#endif // MB8877_TEST_HELPER_H