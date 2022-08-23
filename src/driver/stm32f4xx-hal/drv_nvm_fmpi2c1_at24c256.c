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

#include "drv_nvm_fmpi2c1_at24C256.h"
#include "stm32f4xx_hal.h"

/******************************************************************************
* PRIVATE TYPE DEFINITION
******************************************************************************/

typedef struct PIN_ASSIGN_T {
    GPIO_TypeDef *Port;
    uint16_t      Pin;
    uint8_t       Alternate;
} PIN_ASSIGN;

/******************************************************************************
* PRIVATE DEFINES
******************************************************************************/

#define EEPROM_ADDRESS     (uint16_t)0xA0 /* address of I2C device bit 1..7  */
#define EEPROM_TIMEOUT     (uint32_t)1000 /* timeout for each transfer: 1s   */
#define EEPROM_PAGE_SIZE   (uint16_t)64   /* EEPROM page size                */
#define EEPROM_ACK_POLLING 1000           /* retry on ack polling            */

/* default pin assignment: FMPI2C_SCL -> PD14, FMPI2C1_SDA -> PD15 */
#define FMPI2C1_PIN_SCL_SEL     2
#define FMPI2C1_PIN_SDA_SEL     2

/******************************************************************************
* PRIVATE VARIABLES
******************************************************************************/

static PIN_ASSIGN FMPI2C1Pin_Scl[] = {
    { GPIOC, GPIO_PIN_6,  GPIO_AF4_FMPI2C1 },  /* #0: PC6 */ 
    { GPIOD, GPIO_PIN_12, GPIO_AF4_FMPI2C1 },  /* #1: PD12 */ 
    { GPIOD, GPIO_PIN_14, GPIO_AF4_FMPI2C1 },  /* #2: PD14 */ 
    { GPIOF, GPIO_PIN_14, GPIO_AF4_FMPI2C1 }   /* #3: PF14 */ 
};
static PIN_ASSIGN FMPI2C1Pin_Sda[] = {
    { GPIOC, GPIO_PIN_7,  GPIO_AF4_FMPI2C1 },  /* #0: PC7 */ 
    { GPIOD, GPIO_PIN_13, GPIO_AF4_FMPI2C1 },  /* #1: PD13 */ 
    { GPIOD, GPIO_PIN_15, GPIO_AF4_FMPI2C1 },  /* #2: PD15 */ 
    { GPIOF, GPIO_PIN_15, GPIO_AF4_FMPI2C1 }   /* #3: PF15 */ 
};

static FMPI2C_HandleTypeDef DrvFPMI2CBus;

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void     DrvNvmInit  (void);
static uint32_t DrvNvmRead  (uint32_t start, uint8_t *buffer, uint32_t size);
static uint32_t DrvNvmWrite (uint32_t start, uint8_t *buffer, uint32_t size);

/******************************************************************************
* PUBLIC VARIABLE
******************************************************************************/

const CO_IF_NVM_DRV STM32F4xx_FMPI2C1_AT24C256_NvmDriver = {
    DrvNvmInit,
    DrvNvmRead,
    DrvNvmWrite
};

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void DrvNvmInit(void)
{
    HAL_StatusTypeDef err;
    GPIO_InitTypeDef gpio = {0};
    RCC_PeriphCLKInitTypeDef pclk = {0};

    /* Peripheral clocks enable */
    pclk.PeriphClockSelection = RCC_PERIPHCLK_FMPI2C1;
    pclk.Fmpi2c1ClockSelection = RCC_FMPI2C1CLKSOURCE_APB;
    err = HAL_RCCEx_PeriphCLKConfig(&pclk);
    if (err != HAL_OK) {
        while(1);    /* error not handled */
    }
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /* FMPI2C1 GPIO Configuration */
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = FMPI2C1Pin_Scl[FMPI2C1_PIN_SCL_SEL].Alternate;
    gpio.Pin       = FMPI2C1Pin_Scl[FMPI2C1_PIN_SCL_SEL].Pin;
    HAL_GPIO_Init(FMPI2C1Pin_Scl[FMPI2C1_PIN_SCL_SEL].Port, &gpio);
    gpio.Alternate = FMPI2C1Pin_Sda[FMPI2C1_PIN_SDA_SEL].Alternate;
    gpio.Pin       = FMPI2C1Pin_Sda[FMPI2C1_PIN_SDA_SEL].Pin;
    HAL_GPIO_Init(FMPI2C1Pin_Sda[FMPI2C1_PIN_SDA_SEL].Port, &gpio);

    DrvFPMI2CBus.Instance = FMPI2C1;
    DrvFPMI2CBus.Init.Timing = 0x0020081B;
    DrvFPMI2CBus.Init.OwnAddress1 = 0;
    DrvFPMI2CBus.Init.AddressingMode = FMPI2C_ADDRESSINGMODE_7BIT;
    DrvFPMI2CBus.Init.DualAddressMode = FMPI2C_DUALADDRESS_DISABLE;
    DrvFPMI2CBus.Init.OwnAddress2 = 0;
    DrvFPMI2CBus.Init.OwnAddress2Masks = FMPI2C_OA2_NOMASK;
    DrvFPMI2CBus.Init.GeneralCallMode = FMPI2C_GENERALCALL_DISABLE;
    DrvFPMI2CBus.Init.NoStretchMode = FMPI2C_NOSTRETCH_DISABLE;
    err = HAL_FMPI2C_Init(&DrvFPMI2CBus);
    if (err != HAL_OK) {
        while(1);    /* error not handled */
    }

    /** Configure Analogue filter */
    err = HAL_FMPI2CEx_ConfigAnalogFilter(&DrvFPMI2CBus, FMPI2C_ANALOGFILTER_ENABLE);
    if (err != HAL_OK) {
        while(1);    /* error not handled */
    }

    /** I2C Fast mode Plus enable */
    HAL_FMPI2CEx_EnableFastModePlus(FMPI2C_FASTMODEPLUS_SCL);
    HAL_FMPI2CEx_EnableFastModePlus(FMPI2C_FASTMODEPLUS_SDA);
}

static uint32_t DrvNvmRead(uint32_t start, uint8_t *buffer, uint32_t size)
{
    HAL_StatusTypeDef err;
    uint32_t  result   = 0;
    uint16_t  memStart = (uint16_t)start;
    uint16_t  memSize  = (uint16_t)size;
    uint8_t  *memData  = buffer;

    while (memSize > EEPROM_PAGE_SIZE) {
        err = HAL_FMPI2C_Mem_Read(
            &DrvFPMI2CBus,
            EEPROM_ADDRESS,
            memStart,
            I2C_MEMADD_SIZE_16BIT,
            memData,
            EEPROM_PAGE_SIZE,
            EEPROM_TIMEOUT);
        if (err != HAL_OK) {
            return (result);
        }
        memStart += EEPROM_PAGE_SIZE;
        memSize  -= EEPROM_PAGE_SIZE;
        memData  += EEPROM_PAGE_SIZE;
        result   += EEPROM_PAGE_SIZE;
    }
    err = HAL_FMPI2C_Mem_Read(
        &DrvFPMI2CBus,
        EEPROM_ADDRESS,
        memStart,
        I2C_MEMADD_SIZE_16BIT,
        memData,
        memSize,
        EEPROM_TIMEOUT);
    if (err != HAL_OK) {
        return (result);
    }
    result += memSize;

    return (result);
}

static uint32_t DrvNvmWrite(uint32_t start, uint8_t *buffer, uint32_t size)
{
    HAL_StatusTypeDef err;
    uint32_t  result   = 0;
    uint16_t  memStart = (uint16_t)start;
    uint16_t  memSize  = (uint16_t)size;
    uint8_t  *memData  = buffer;
    uint16_t  memPart  = 0;

    /* write data up to the next block boundary */
    memPart = EEPROM_PAGE_SIZE - (memStart % EEPROM_PAGE_SIZE);
    if ((memPart < memSize) &&
        (memPart > 0      )) {
        err = HAL_FMPI2C_Mem_Write(
            &DrvFPMI2CBus,
            EEPROM_ADDRESS,
            memStart,
            I2C_MEMADD_SIZE_16BIT,
            memData,
            memPart,
            EEPROM_TIMEOUT);
        if (err != HAL_OK) {
            return (result);
        }
        memStart += memPart;
        memData  += memPart;
        memSize  -= memPart;
        result   += memPart;
        /* ACK polling during EEPROM internal write operations */
        err = HAL_FMPI2C_IsDeviceReady(
            &DrvFPMI2CBus,
            EEPROM_ADDRESS,
            EEPROM_ACK_POLLING,
            EEPROM_TIMEOUT);
        if (err != HAL_OK) {
            return (result);
        }
    }

    /* write all full blocks */
    while (memSize > EEPROM_PAGE_SIZE) {
        err = HAL_FMPI2C_Mem_Write(
            &DrvFPMI2CBus,
            EEPROM_ADDRESS,
            memStart,
            I2C_MEMADD_SIZE_16BIT,
            memData,
            EEPROM_PAGE_SIZE,
            EEPROM_TIMEOUT);
        if (err != HAL_OK) {
            return (result);
        }
        memStart += EEPROM_PAGE_SIZE;
        memData  += EEPROM_PAGE_SIZE;
        memSize  -= EEPROM_PAGE_SIZE;
        result   += EEPROM_PAGE_SIZE;
        /* ACK polling during EEPROM internal write operations */
        err = HAL_FMPI2C_IsDeviceReady(
            &DrvFPMI2CBus,
            EEPROM_ADDRESS,
            EEPROM_ACK_POLLING,
            EEPROM_TIMEOUT);
        if (err != HAL_OK) {
            return (result);
        }
    }

    /* write the last partly filled block */
    if (memSize > 0) {
        err = HAL_FMPI2C_Mem_Write(
            &DrvFPMI2CBus,
            EEPROM_ADDRESS,
            memStart,
            I2C_MEMADD_SIZE_16BIT,
            memData,
            memSize,
            EEPROM_TIMEOUT);
        if (err != HAL_OK) {
            return (result);
        }
        result += memSize;
        /* ACK polling during EEPROM internal write operations */
        err = HAL_FMPI2C_IsDeviceReady(
            &DrvFPMI2CBus,
            EEPROM_ADDRESS,
            EEPROM_ACK_POLLING,
            EEPROM_TIMEOUT);
        if (err != HAL_OK) {
            return (result);
        }
    }
    return (result);
}
