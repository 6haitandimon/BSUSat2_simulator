#include "../../Core/Inc/dma.h"
#include "../../Core/Inc/tim.h"
#include "leads.h"

#define DATA_LEN ((NUM_LEDS * 24) + 2)


uint8_t pwmData[48];
uint8_t datasentflag = 0;

//todo сделать реализацию через CMSIS ,что то подобноеw На ws2812b_init(TIM_TypeDef * tim,частоты шины-?,частота шима,Порт)
void ws2812b_init(){
    MX_DMA_Init();
    MX_TIM3_Init();
}

void WS2812_Send(int pixelNum, uint8_t red, uint8_t green, uint8_t blue){

    int offset = 0;
    uint32_t color = ((uint32_t)green << 0) | ((uint32_t)red << 8) | ((uint32_t)blue << 16) ;
    for (int i = 0; i <= pixelNum; i++)
    {
        offset = i * 24;
        for (int j = offset + 23; j >= offset; j--)
        {
            if (color & (1 << (j - offset)))
            {
                pwmData[j] = TIME_1_H;
            }
            else
            {
                pwmData[j] = TIME_0_H;
            }
        }
    }

    HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)pwmData,98);
    while (!datasentflag){};
    datasentflag = 0;
    WS2812_Reset();
}
void WS2812_Reset() {

    uint32_t tmp[50];
    for (int i=49; i>=0; i--)
    {
        tmp[i] = 0x0;
    }
    //tmp[0] = RESET_TIME;

    HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, tmp, 52);

    // Дожидаемся окончания передачи
    while (!datasentflag) {};
    datasentflag = 0;

}
/*void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
    datasentflag = 1;
}*/
