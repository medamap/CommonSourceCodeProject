/*
    Phase 36: Simple Force Interrupt Test
*/

#include <stdio.h>
#include <string.h>
#include "test_framework.h"
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

void test_force_interrupt_basic() {
    printf("=== Testing Basic Force Interrupt ===\n");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    
    // Open disk
    fdc.open_disk(0, nullptr, 0);
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Start a long command (RESTORE)
    fdc.write_io8(0, 0x03);  // RESTORE command
    event.advance_clock(100);
    
    // Check BUSY
    uint32_t status = fdc.read_io8(0);
    printf("Status during RESTORE: 0x%02X (BUSY=%d)\n", status, (status & 0x01) ? 1 : 0);
    
    // Issue Force Interrupt with no IRQ (D0)
    printf("\nIssuing Force Interrupt (0xD0 - no IRQ)...\n");
    fdc.write_io8(0, 0xD0);
    event.advance_clock(100);
    
    status = fdc.read_io8(0);
    printf("Status after Force Interrupt: 0x%02X (BUSY=%d)\n", status, (status & 0x01) ? 1 : 0);
    
    // Test immediate IRQ (D8)
    printf("\nIssuing Force Interrupt (0xD8 - immediate IRQ)...\n");
    fdc.write_io8(0, 0xD8);
    event.advance_clock(20);  // Allow for IRQ delay
    
    status = fdc.read_io8(0);
    printf("Status after immediate IRQ: 0x%02X\n", status);
    
    // Test index pulse IRQ (D4)
    printf("\nIssuing Force Interrupt (0xD4 - index pulse IRQ)...\n");
    fdc.write_io8(0, 0xD4);
    
    // Wait for index pulse (simulate disk rotation)
    event.advance_clock(200000);  // 200ms should be enough for index
    
    status = fdc.read_io8(0);
    printf("Status after index pulse wait: 0x%02X\n", status);
    
    printf("\nBasic Force Interrupt test complete.\n");
}

int main() {
    printf("Phase 36: Force Interrupt Implementation Test\n");
    printf("==========================================\n");
    
    test_force_interrupt_basic();
    
    printf("\nTest completed successfully.\n");
    return 0;
}