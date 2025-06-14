/*
    MB8877 Force Interrupt Debug Test
    
    Author : Claude AI Assistant
    Date   : 2025.01.14
*/

#include "test_framework.h"
#include "mock_environment.h"
#include "../../../src/vm/mb8877_compat.h"

void test_force_interrupt_debug() {
    printf("\n=== Force Interrupt Debug Test ===\n");
    
    MockEMU emu;
    MockVM vm(&emu);
    MockEVENT event(&vm, &emu);
    MockDISK mock_disk(&vm, &emu);
    
    MB8877 fdc(&vm, &emu);
    fdc.set_context_event_manager(&event, 0, 0, 0);
    fdc.initialize();
    fdc.reset();
    fdc.write_signal(SIG_MB8877_MOTOR, 1, 1);
    
    // Test 1: Force Interrupt with no conditions (0xD0)
    printf("\n--- Test 1: Force Interrupt 0xD0 (no conditions) ---\n");
    printf("Before: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    fdc.write_io8(0, 0xD0);
    
    printf("After write 0xD0: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    // Advance clock to let any events fire
    event.advance_clock(100);
    
    printf("After 100us: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    // Test 2: Force Interrupt with immediate IRQ (0xD8)
    printf("\n--- Test 2: Force Interrupt 0xD8 (immediate IRQ) ---\n");
    printf("Before: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    fdc.write_io8(0, 0xD8);
    
    printf("After write 0xD8: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    // Advance clock to let EVENT_IRQ fire
    event.advance_clock(100);
    
    printf("After 100us: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    // Read status to clear IRQ
    uint32_t status = fdc.read_io8(0);
    printf("Status read: 0x%02X\n", status);
    printf("After status read: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    // Test 3: Another Force Interrupt with no conditions
    printf("\n--- Test 3: Force Interrupt 0xD0 again ---\n");
    printf("Before: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    fdc.write_io8(0, 0xD0);
    
    printf("After write 0xD0: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
    
    event.advance_clock(100);
    
    printf("After 100us: get_intr_ack() = %s\n", fdc.get_intr_ack() ? "true" : "false");
}

int main() {
    test_force_interrupt_debug();
    return 0;
}