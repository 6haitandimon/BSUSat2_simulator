#include "interrupt.h"

void MCPCallback(uint gpio, uint32_t events){
    can_frame rx;
    printf("irq\n");
    if(can0.readMessage(&rx) == MCP2515::ERROR_OK)
        printf("id: %d\n", rx.can_id);
    can0.clearInterrupts();
    return;
}

namespace INTERRUPT{
    void MCPInterruptSetup(){

        gpio_set_irq_enabled_with_callback(MCPInterruptPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, MCPCallback);
        can0.clearInterrupts();
        return;
    }
}