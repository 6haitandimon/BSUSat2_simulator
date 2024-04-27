#include <memory.h>
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

#include "BH1750.h"
#include "software_i2c.h"

void SystemClock_Config(void);

#define RxSIZE  11
uint8_t dataRegister[10] = {0,0};
uint8_t RxData[RxSIZE];
uint8_t rxcount=0;
uint8_t txcount=0;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

  if (HAL_I2C_EnableListen_IT(&hi2c1) != HAL_OK)
  {
      Error_Handler();
  }

  sw_i2cT hi2c = SW_I2C_INIT(GPIOA,GPIO_PIN_1,GPIOA,GPIO_PIN_0);
  BH1750_t obj = BH1750_Init(&hi2c,true); //generate object

  while (1){
      dataRegister[0] = BH1750_get_value(&obj);
      HAL_Delay(5000);
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_I2C_ListenCpltCallback (I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode){
    if (TransferDirection == I2C_DIRECTION_TRANSMIT){
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData+rxcount, 1, I2C_FIRST_FRAME);
    }else{
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, dataRegister+txcount, 1, I2C_FIRST_FRAME);
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c){
    txcount++;
    HAL_I2C_Slave_Seq_Transmit_IT(hi2c, dataRegister+txcount, 1, I2C_NEXT_FRAME);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    rxcount++;
    if (rxcount < RxSIZE){
        if (rxcount == RxSIZE-1){
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData+rxcount, 1, I2C_LAST_FRAME);
        }else{
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData+rxcount, 1, I2C_NEXT_FRAME);
        }
    }

}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    uint32_t errorcode = HAL_I2C_GetError(hi2c);

    if (errorcode == 4)  // AF error
    {
        if (txcount == 0)  // error is while slave is receiving
        {
            rxcount = 0;  // Reset the rxcount for the next operation
        }
        else // error while slave is transmitting
        {
            txcount = 0;  // Reset the txcount for the next operation
        }
    }

    else if (errorcode == 1)  // BERR Error
    {
        HAL_I2C_DeInit(hi2c);
        HAL_I2C_Init(hi2c);
        memset(RxData,'\0',RxSIZE);  // reset the Rx buffer
        rxcount =0;  // reset the count
    }

    HAL_I2C_EnableListen_IT(hi2c);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }

}

#ifdef  USE_FULL_ASSERT
#endif /* USE_FULL_ASSERT */
