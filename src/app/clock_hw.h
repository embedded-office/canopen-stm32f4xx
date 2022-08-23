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

#ifndef CLOCK_HW_H_
#define CLOCK_HW_H_

#ifdef __cplusplus               /* for compatibility with C++ environments  */
extern "C" {
#endif

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "stm32f4xx_hal.h"              /* for connecting interrupts         */

                                        /* select application drivers:       */
#include "drv_can_can1.h"               /* CAN driver (CAN1)                 */
#include "drv_timer_tim2.h"             /* Timer driver (TIM2)               */
#include "drv_nvm_fmpi2c1_at24c256.h"   /* NVM driver (AT24C256 via FMPI2C1) */

/******************************************************************************
* PUBLIC SYMBOLS
******************************************************************************/

extern struct CO_IF_DRV_T AppDriver;

#ifdef __cplusplus               /* for compatibility with C++ environments  */
}
#endif

#endif /* CLOCK_HW_H_ */
