#include "interrupt.h"

void canAnswer(can_frame rx){

}

void MCPCallback(uint gpio, uint32_t events){
    can_frame rx;
    printf("irq\n");
//    can0.clearInterrupts();
    if(can0.readMessage(&rx) == MCP2515::ERROR_OK){
        printf("id: %d\n", rx.can_id);
        printf("dlc: %d\n", rx.can_dlc);
        for(int i = 0; i < rx.can_dlc; i++)
            printf("data byte %d: %d\n", i, rx.data[i]);

        canAnswer(rx);
    }else{
        can0.clearInterrupts();
    }
    return;
}



namespace INTERRUPT{
    void MCPInterruptSetup(){
//        can0.clearInterrupts();
        gpio_init(MCPInterruptPin);
        gpio_set_dir(MCPInterruptPin, GPIO_IN);
        gpio_set_irq_enabled_with_callback(MCPInterruptPin, GPIO_IRQ_EDGE_FALL, true, MCPCallback);
//        can0.clearInterrupts();
        return;
    }
}