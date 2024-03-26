#include "si1143.h"

si1143_t SI1143_Init(sw_i2cT *hi2c) {
    si1143_t obj;
    _delay(25); // Start-Up Time. Table №1
    if(obj.status = I2C_IsDeviceReady(hi2c,0x5A,0,0) != OK)
        return obj;
    obj.i2c_addres = I2C_ADDRES;

    if( obj.status = SI1143_ON(&obj) != OK) { obj.mode = Init; }
    else{ obj.mode = StandBy; }

    obj.data = (si1143Data_t){0};
    return obj;
}

StatusTypeDef SI1143_ON(si1143_t *obj) {
    u16 initCmd[2]={};

    //The system must write the value 0x17 to this register for proper Si114x operation.
    initCmd[0] = HW_KEY; initCmd[1] = 0x17;
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    // turn on int
    initCmd[0] = INT_CFG; initCmd[1] = 0x03;
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    // turn on int on PS3
    initCmd[0] = IRQ_ENABLE; initCmd[1] = 0x10;
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    //int on ps3 meas
    initCmd[0] = IRQ_MODE2; initCmd[1] = 0x01;
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    // 10 ms meas rate
    initCmd[0] = MEAS_RATE; initCmd[1] = 0x84;
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    // Set ALS_RATE 1:1 with MEAS
    initCmd[0] = ALS_RATE; initCmd[1] = 0x08;
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    // Set PS_RATE 1:1 with MEAS
    initCmd[0] = PS_RATE;
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    // Current setting for LEDs pulsed
    initCmd[0] = PS_LED21; initCmd[1] = 0xF0; // todo яки ток нужен, нужно ли менять его в процессе работы?
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    initCmd[0] = PS_LED3; initCmd[1] = 0x0F; // todo яки ток нужен, нужно ли менять его в процессе работы?
    if( obj->status = writeInReg(obj, initCmd, 2) != OK ){ return obj->status; }
    // Enabled all meas
    // [7] - Reserved, [6] - EN_AUX, [5] - EN_ALS_IR, [4] - EN_ALS_VIS,
    // [3] - Reserved, [2] - EN_PS3, [1] - EN_PS2,  [0] - EN_PS1
    initCmd[0] = CHLIST; initCmd[1] = 0b01110111;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    // Increasing the PS_ADC_GAIN value by one increases the irLED pulse pulse and ADC run time by a factor of 2^PS_ADC_GAIN for all PS measurements.
    // 0x0: ADC Clock is divided by 1
    // 0x4: ADC Clock is divided by 16
    // 0x5: ADC Clock is divided by 32
    initCmd[0] = PS_ADC_GAIN; initCmd[1] = 0x00;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    // Specifies the LED pin driven during the PS2&PS1 Measurement
    // 000: No Led Driver
    // xx1: LED1 Drive Enabled
    // x1x: LED2 Drive Enabled (Si1142 and Si1143 only. Clear for Si1141)
    // 1xx: LED3 Drive Enabled (Si1143 only. Clear for Si1141 and Si1142)
    initCmd[0] = PSLED12_SELECT; initCmd[1] = 0b00100010;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    // Enable Led driver 3
    initCmd[0] = PSLED3_SELECT; initCmd[1] = 0b00000010;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    // select photodiode
    // 0x00: Small IR Photodiode
    // 0x02: Visible Photodiode
    // 0x03: Large IR Photodiode
    // 0x06: No Photodiode
    // 0x25: GND voltage todo ??????
    // 0x65: Temperature todo ??????
    initCmd[0] = PS1_ADCMUX; initCmd[1] = 0x03;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    initCmd[0] = PS2_ADCMUX; initCmd[1] = 0x03;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    initCmd[0] = PS3_ADCMUX; initCmd[1] = 0x03;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    // Recovery period the ADC takes before making a PS measurement.
    // 3:0 Reserved Always set to 0.
    // Reset value = 0111 0000
    initCmd[0] = PS_ADC_COUNTER; initCmd[1] = 0b01110000;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    // 0x13: Starts/Restarts an autonomous PS Loop
    // 0x14: Starts/Restarts an autonomous ALS Loop
    // 0x15: Starts/Restarts autonomous ALS and PS loop
    // starts an autonomous read loop
    initCmd[0] = COMMAND; initCmd[1] = 0x15;
    if( obj->status = writeParam(obj, initCmd, 2) != OK ){ return obj->status; }
    return obj->status;
}

StatusTypeDef writeInReg(si1143_t *obj, u16 *initCmd, uint8_t size) {
    return I2C_Master_Transmit(obj->hi2c,obj->i2c_addres,initCmd,size,10);
}

StatusTypeDef writeParam(si1143_t *obj, uint16_t *initCmd, uint8_t size) {
    u16 buffer[3]= {PARAM_WR, initCmd[1], 0xA0 | initCmd[0]}; // PARAM_SET 101aaaaa
    return obj->status = I2C_Master_Transmit(obj->hi2c,obj->i2c_addres,buffer, size + 1 ,10); // todo size
}
/**
 *
 * @param obj
 * @param type  = 1 LED DATA
 * @return
 */
StatusTypeDef readSensor(si1143_t *obj, u8 type) {
    u16 pData[3];
    if(type && 0x01) {
        if( obj->status = I2C_Master_Transmit(obj->hi2c, I2C_ADDR, (uint16_t *) PS1_DATA0, 1, 10) != OK){ return obj->status; }
        if(obj->status = getLedData(obj, pData)){ return obj->status; }
        obj->data.redLed += pData[0];
        obj->data.ir1Led += pData[1];
        obj->data.ir2Led += pData[2];
    }
    if(type && 0x02){
        if( obj->status = I2C_Master_Transmit(obj->hi2c, I2C_ADDR, (uint16_t *) ALS_VIS_DATA0, 1, 10) != OK){ return obj->status; }
        if(obj->status = getAlsData(obj, pData)){ return obj->status; }
        obj->data.alsVisible += pData[0];
        obj->data.alsIR += pData[1];
    }
    return OK;
}

StatusTypeDef getLedData(si1143_t *obj, u16 *pData) {
    u16 tempData[2]={};
    for(u8 i = 0 ; i < 3; i++){
        if( obj->status = I2C_Master_Receive(obj->hi2c, I2C_ADDR, tempData, 2, 10) != OK)
            return obj->status;
        pData[i] = tempData[0] + (tempData[1] << 8);
        tempData[0] = 0; tempData[1] = 0; //todo: del
    }
    return OK;
}

StatusTypeDef getAlsData(si1143_t *obj, uint16_t *pData) {
    u16 tempData[2]={};
    for(u8 i = 0 ; i < 2 ;i++){
        if( obj->status = I2C_Master_Receive(obj->hi2c, I2C_ADDR, tempData, 2, 10) != OK)
            return obj->status;
        pData[i] = tempData[0] + (tempData[1] << 8);
        tempData[0] = 0; tempData[1] = 0; //todo: del?
    }
    return OK;
}
