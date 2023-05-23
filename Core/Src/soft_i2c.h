#ifndef SOFT_I2C_H
#define SOFT_I2C_H

//программный i2c

#include <stdint.h>
//#include "stm32f4xx_hal_gpio.h"
#include "main.h"

#include "stm32f4xx_hal.h"

#define SCL_PIN GPIO_PIN_6
#define SCL_PORT GPIOB

#define SDA_PIN GPIO_PIN_7
#define SDA_PORT GPIOB

#define __SCL_HIGH() 	HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_SET)
#define __SCL_LOW() 	HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_RESET)
//#define SDA_IN 		SDA_PORT->MODER &=  ~(0x3 << SDA_PIN * 2);
//#define SDA_OUT		SDA_PORT->MODER |= (0x1 << (SDA_PIN * 2));
#define __SDA_HIGH()	HAL_GPIO_WritePin(SDA_PORT, SDA_PIN, GPIO_PIN_SET)
#define __SDA_LOW()		HAL_GPIO_WritePin(SDA_PORT, SDA_PIN, GPIO_PIN_RESET)
#define __SDA_READ()  	 HAL_GPIO_ReadPin(SDA_PORT, SDA_PIN);

void delay_us7(uint16_t us);

void SDA_OUT();
void SDA_IN();

void SoftI2cInit();               // Инициализация шины
void SoftI2cStart();        // Генерация условия старт
void SoftI2cRestart();      // Генерация условия рестарт
void SoftI2cStop();       // Генерация условия стоп  
uint8_t SoftI2cSendByte(uint8_t data);      //Передать байт (вх. аргумент передаваемый байт) (возвращает 0 - АСК, 1 - NACK) 
uint8_t SoftI2cGetByte(uint8_t last_byte);  //Принять байт (если последний байт то входной аргумент = 1, если будем считывать еще то 0)(возвращает принятый байт)

void HAL_I2C_Mem_Write(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
void HAL_I2C_Mem_Read(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif //SOFT_I2C_H
