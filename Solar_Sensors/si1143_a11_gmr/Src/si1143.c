#include "si1143.h"

si1143_t initSi1143(sw_i2cT *hi2c) {
    si1143_t obj;
    _delay(25); // Start-Up Time. Table №1
    if(obj.status = I2C_IsDeviceReady(hi2c,0x5A,0,0) != OK)
        return obj;
    obj.i2c_addres = I2C_ADDRES;

    if( obj.status = SI1143_ON(&obj) != OK) {
        obj.mode = Init;
    }else{
        obj.mode = StandBy;
    }

    return obj;
}

StatusTypeDef SI1143_ON(si1143_t *obj) {
    u16 initCmd[2] = {HW_KEY,0x17};
    obj->status = I2C_Master_Transmit(obj->hi2c,obj->i2c_addres,initCmd,2,10);
    initCmd[0] = PS_LED21; initCmd[1] = 0xFF;
    obj->status = I2C_Master_Transmit(obj->hi2c,obj->i2c_addres,initCmd,2,10);
    initCmd[0] = PS_LED3; initCmd[1] = 0x0F;
    return obj->status;
}
