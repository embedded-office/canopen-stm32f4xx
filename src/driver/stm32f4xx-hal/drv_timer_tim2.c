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

#include "drv_timer_tim2.h"
#include "stm32f4xx_hal.h"

/******************************************************************************
* PRIVATE VARIABLES
******************************************************************************/

static TIM_HandleTypeDef DrvTimer2;

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void     DrvTimerInit   (uint32_t freq);
static void     DrvTimerStart  (void);
static uint8_t  DrvTimerUpdate (void);
static uint32_t DrvTimerDelay  (void);
static void     DrvTimerReload (uint32_t reload);
static void     DrvTimerStop   (void);

/******************************************************************************
* PUBLIC VARIABLE
******************************************************************************/

const CO_IF_TIMER_DRV STM32F4xx_TIM2_TimerDriver = {
    DrvTimerInit,
    DrvTimerReload,
    DrvTimerDelay,
    DrvTimerStop,
    DrvTimerStart,
    DrvTimerUpdate
};

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/

/* ST HAL Timer2 Interrupt Handler */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&DrvTimer2);
}

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void DrvTimerInit(uint32_t freq)
{
    /* For simplicity, we implement a fixed frequency of 1MHz:
       - the input frequency TIM2_CLK (APB1) is: 90 MHz
       - with a prescaler of: 90, we get a counting frequency of: 1MHz
       - in other words we get a tick every: dt = 1us
       - with the timer width 32bit we get: max. time = 4294s (1h 11min 34s)
     */
    if (freq != 1000000) {
        while(1);    /* see comments above */
    }

    /* Peripheral clocks enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* configure timer to: up-counting timer with tick rate 1MHz,
     * with overflow interrupt at given (unbuffered) autoreload register
     * (note: the timer reloads to 0 on this overflow)
     */
    DrvTimer2.Instance = TIM2;
    DrvTimer2.Init.Prescaler = 90;
    DrvTimer2.Init.CounterMode = TIM_COUNTERMODE_UP;
    DrvTimer2.Init.Period = 4294967295;
    DrvTimer2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    DrvTimer2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&DrvTimer2) != HAL_OK) {
        while(1);    /* error not handled */
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&DrvTimer2, &sClockSourceConfig) != HAL_OK) {
        while(1);    /* error not handled */
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&DrvTimer2, &sMasterConfig) != HAL_OK) {
        while(1);    /* error not handled */
    }

    /* clear pending update interrupt */
    __HAL_TIM_CLEAR_FLAG(&DrvTimer2, TIM_FLAG_UPDATE);

    /* enable timer interrupts */
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

static void DrvTimerStart(void)
{
    /* start the hardware timer counting */
    HAL_TIM_Base_Start_IT(&DrvTimer2);
}

static uint8_t DrvTimerUpdate(void)
{
    /* a hardware timer interrupt is always an elapsed event */
    return 1u;
}

static uint32_t DrvTimerDelay(void)
{
    uint32_t current = __HAL_TIM_GET_COUNTER(&DrvTimer2);
    uint32_t reload = __HAL_TIM_GET_AUTORELOAD(&DrvTimer2);

    /* return remaining ticks until interrupt occurs */
    return (reload - current);
}

static void DrvTimerReload(uint32_t reload)
{
    /* configure the next hardware timer interrupt */
    __HAL_TIM_SET_AUTORELOAD(&DrvTimer2, reload);
}

static void DrvTimerStop(void)
{
    /* stop the hardware timer counting */
    HAL_TIM_Base_Stop_IT(&DrvTimer2);
}
