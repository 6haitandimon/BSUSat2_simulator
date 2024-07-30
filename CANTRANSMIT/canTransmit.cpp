#include "canTransmit.h"

namespace CAN{
    union canByte{
        uint16_t data;
        struct{
            uint8_t byte[2];
        };
    };

//    static uint8_t busyByte = 1;
//    static const uint8_t freeByte = 8;

    int8_t CanPreparation(can_frame &canFrame, uint8_t needToByte, uint16_t data){
        canByte canData;
        canData.data = data;
//        if(busyByte + needToByte > freeByte){
//            busyByte = 1;
//            return -1;
//        }
        for(uint8_t index = 1; index < needToByte + 1; index++){
            canFrame.data[index] = canData.byte[index - 1];
        }
        canFrame.can_dlc += needToByte;
        return 0;
    }
}