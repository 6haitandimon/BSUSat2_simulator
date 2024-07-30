#include "interrupt.h"

void canAnswer(can_frame rx){
//    can0.clearTXInterrupts();
    can_frame tx;
    tx.can_id = 0x01;
    tx.can_dlc = 1;
    tx.data[0] = 62;
    if(rx.data[1] == 9){
        if(can0.sendMessage(&tx) ==MCP2515::ERROR_OK){
            printf("Answer done\n");
            printf("Count frame: %d\n", tx.data[0]);
        }
        busy_wait_ms(100);
//        sleep_ms(2000);
        uint32Time unixTimeData;
        unixTimeData.data = rtc.dataTimeUnix() - last_reset_time;

        MatherBoardTelemtry[12] = unixTimeData.data1;
        MatherBoardTelemtry[13] = unixTimeData.data2;

        unixTimeData.data = rtc.dataTimeUnix();
        MatherBoardTelemtry[20] = unixTimeData.data1;
        MatherBoardTelemtry[21] = unixTimeData.data2;

        for(int index = 0; index < tx.data[0]; index++){
            can_frame frame;
            frame.can_dlc = 1;
            frame.data[0] = index + 1;
            CAN::CanPreparation(frame, 2, MatherBoardTelemtry[index]);
            uint8_t error = -1;
            while(error != MCP2515::ERROR::ERROR_OK){
                error = can0.sendMessage(&frame);
                busy_wait_ms(100);
                }
            printf("can frame number %d send done", index);
        }
    }
    return;
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