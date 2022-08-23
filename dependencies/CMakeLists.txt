#*****************************************************************************
#   Copyright 2020 Embedded Office GmbH & Co. KG
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#******************************************************************************

#---
# Add the CANopen Stack project from Embedded Office
#
set(CO_TARGET   "canopen-stack")
set(CO_PROJECT  "embedded-office/canopen-stack")
set(CO_VERSION  "4.4.0")
CPMAddPackage(
  NAME    canopen-stack
  URL     https://github.com/${CO_PROJECT}/releases/download/v${CO_VERSION}/${CO_TARGET}-src.zip
  VERSION ${CO_VERSION}
)

#---
# Add the ST Microelectronics HAL drivers with some small patches
# - to get a smaller package size
# - eliminate compilation errors with -Wpedantic
#
set(HAL_TARGET   "stm32f4xx-hal")
set(HAL_PROJECT  "embedded-office/STM32CubeF4")
set(HAL_VERSION  "1.27.1-p2")
#
# configure component build for:
#
set(HAL_DEVICE   "STM32F446xx")
set(HAL_CONFIG   "${CMAKE_SOURCE_DIR}/src/config")
CPMAddPackage(
  NAME    ${HAL_TARGET}
  URL     https://github.com/${HAL_PROJECT}/releases/download/v${HAL_VERSION}/${HAL_TARGET}-src.zip
  VERSION ${HAL_VERSION}
  OPTIONS
    "STM_HAL_CONF_DIR ${HAL_CONFIG}"
    "STM_HAL_DEVICE ${HAL_DEVICE}"
    "STM_USE_HAL_DRIVER true"
    "STM_USE_FULL_LL_DRIVER true"
    "STM_USE_FULL_ASSERT false"
)
