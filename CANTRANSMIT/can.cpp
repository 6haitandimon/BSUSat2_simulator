#include "can.h"

namespace CAN{
    union canByte{
        float data;
        struct{
            uint8_t byte1[4];
        };
    };

    static uint8_t busyByte = 1;
    static const uint8_t freeByte = 7;

    int CanPreparation(can_frame &canFrame, uint8_t needToByte, float data){
        canByte canData;
        canData.data = data;
        if(busyByte + needToByte > freeByte)
            return -1;
        for(uint8_t index = busyByte; index < needToByte + busyByte; index++){
            canFrame.data[index] = index - busyByte;
        }
    }
}