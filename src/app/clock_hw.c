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

#include "clock_hw.h"
#include "clock_app.h"

/******************************************************************************
* PUBLIC VARIABLES
******************************************************************************/

/* Select the drivers for your application. For possible
 * selections, see the directory /drivers.
 */
struct CO_IF_DRV_T AppDriver = {
    &STM32F4xx_CAN1_CanDriver,
    &STM32F4xx_TIM2_TimerDriver,
    &STM32F4xx_FMPI2C1_AT24C256_NvmDriver
};

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/

/* ST HAL CAN receive interrupt callback */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    /* process CAN frame with CANopen protocol */
    if (hcan->Instance == CAN1) {
        CONodeProcess(&Clk);
    };
}

/* ST HAL TIM2 overflow interrupt callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htmr)
{
    /* collect elapsed timed actions */
    if (htmr->Instance == TIM2) {
        COTmrService(&Clk.Tmr);
    }
}
