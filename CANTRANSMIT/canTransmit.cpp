#include "canTransmit.h"

namespace CAN{
    union canByte{
        float data;
        struct{
            uint8_t byte[4];
        };
    };

//    static uint8_t busyByte = 1;
//    static const uint8_t freeByte = 8;

    int CanPreparation(can_frame &canFrame, uint8_t needToByte, float data){
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