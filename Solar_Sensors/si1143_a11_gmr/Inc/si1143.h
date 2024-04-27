#ifndef SI1143_H
#define SI1143_H

#include "bh1750_stm32_port.h"
#define  I2C_ADDRES 0x5A;

typedef enum {
    Init = 0x0,
    StandBy,
    Autonomous_Operation,
    Forced_Conversion,
    ALWAYS_ACTIVE
}ModeTypeDef;

enum I2C_REG{
    PART_ID = 0x0,
    REV_ID,
    SEQ_ID,
    INT_CFG,
    IRQ_ENABLE,
    IRQ_MODE1,
    IRQ_MODE2,
    HW_KEY,
    MEAS_RATE,
    ALS_RATE,
    PS_RATE,
    ALS_LOW_TH0,
    ALS_LOW_TH1,
    ALS_HI_TH0,
    ALS_HI_TH1,
    PS_LED21,
    PS_LED3,
    PS1_TH0,
    PS1_TH1,
    PS2_TH0,
    PS2_TH1,
    PS3_TH0,
    PS3_TH1,
    PARAM_WR,
    COMMAND,//0x18

    RESPONSE = 0x20,
    IRQ_STATUS,
    ALS_VIS_DATA0,
    ALS_VIS_DATA1,
    ALS_IR_DATA0,
    ALS_IR_DATA1,
    PS1_DATA0,
    PS1_DATA1,
    PS2_DATA0,
    PS2_DATA1,
    PS3_DATA0,
    PS3_DATA1,
    AUX_DATA0,
    AUX_DATA1,
    PARAM_RD, // 0x2E

    CHIP_STAT = 0x30,
    ANA_IN_KEY1 = 0x3B,// 0x3B – 0x3E
    ANA_IN_KEY2,
    ANA_IN_KEY3,
    ANA_IN_KEY4
};

enum RAM_PARAMETERS{
    I2C_ADDR = 0x0,
    CHLIST,
    PSLED12_SELECT,
    PSLED3_SELECT,
    Reserved_1,
    PS_ENCODING,
    ALS_ENCODING,
    PS1_ADCMUX,
    PS2_ADCMUX,
    PS3_ADCMUX,
    PS_ADC_COUNTER,
    PS_ADC_GAIN,
    PS_ADC_MISC,
    Reserved_2,
    ALS_IR_ADCMUX,
    AUX_ADCMUX,
    ALS_VIS_AD_C_COUNTER,
    ALS_VIS_ADC_GAIN,
    ALS_VIS_ADC_MISC,
    Reserved_3,
    Reserved_4_1,
    Reserved_4_2,
    ALS_HYST,
    PS_HYST,
    PS_HISTORY,
    ALS_HISTORY,
    ADC_OFFSET,
    Reserved_5,
    LED_REC,
    ALS_IR_AD_C_COUNTER,
    ALS_IR_ADC_GAIN,
    ALS_IR_ADC_MISC,
};
typedef struct si1143Data{
    u16 redLed;
    u16 ir1Led;
    u16 ir2Led;
    u16 alsIR;
    u16 alsVisible;
}si1143Data_t;

typedef struct si1143{
    sw_i2c_t * hi2c;
    u8 i2c_addres;
    si1143Data_t data;
    ModeTypeDef mode;
    StatusTypeDef status;
}si1143_t;

si1143_t SI1143_Init(sw_i2c_t * hi2c);
StatusTypeDef SI1143_ON(si1143_t * obj);
StatusTypeDef writeInReg(si1143_t * obj,u16 * initCmd,u8 size);
StatusTypeDef writeParam(si1143_t * obj,u16 * initCmd,u8 size);
StatusTypeDef readSensor(si1143_t * obj, u8 type);

StatusTypeDef getLedData(si1143_t * obj,u16 * pData);
StatusTypeDef getAlsData(si1143_t * obj,u16 * pData);
#endif //SI1143_H
