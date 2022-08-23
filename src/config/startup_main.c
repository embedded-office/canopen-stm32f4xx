/******************************************************************************
   Copyright 2020 Embedded Office GmbH & Co. KG

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
******************************************************************************/

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "stm32f4xx_hal.h"
#include "clock_app.h"

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void SystemClock_Config(void);

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/

void _init(void) {;}         /* stub function, called in __libc_init_array() */

void SysTick_Handler(void)             /* ST HAL system tick as 1ms timebase */
{
    HAL_IncTick();
}

void HAL_MspInit(void)            /* ST HAL lowlevel initialization callback */
{
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
}

int main(void)                    /* main entry point for controller startup */
{
    HAL_Init();                          /* must be the first call in main() */
    SystemClock_Config();         /* setup the clock tree (activate the PLL) */

    AppStart();                 /* ok, we are ready to start the application */
}

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

/* This function is generated with STM32CubeMX with the default clock
 * settings for the STM32F446 microcontroller. When you intend to port
 * this example application to a different microcontroller, please
 * don't forget to check (or re-create) this essential clock setup!
 */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    HAL_StatusTypeDef ret = HAL_OK;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    /* Input frequency HSE = 25MHz */
    /* SYSCLK = ((HSE / 15) * 216) / 2 = 180 MHz */
    RCC_OscInitStruct.PLL.PLLM = 15;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if (ret != HAL_OK) {
        while(1);    /* error not handled */
    }

    /* Activate the Over-Drive mode */
    ret = HAL_PWREx_EnableOverDrive();
    if (ret != HAL_OK) {
        while(1);    /* error not handled */
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; /* SYSCLK    : 180 MHz */
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        /* FCLK, HCLK: 180 MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;         /* APB1: 45 MHz, APB1-TMR:  90 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;         /* APB2: 90 MHz, APB2-TMR: 180 MHz */
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
    if (ret != HAL_OK) {
        while(1);    /* error not handled */
    }
}
