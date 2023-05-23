#ifndef SOFT_I2C_V2_H
#define SOFT_I2C_V2_H

//программный i2c версия 2

#include <stdint.h>

//для доступа к регистрам
#include "stm32f407xx.h"

#define PIN_SDA 7
#define PIN_SCL 6
#define PORT_SDA GPIOB
#define PORT_SCL GPIOB

#define DELAY_US 5

//функции для определения платофрмы
static inline void SDA_LOW();
static inline void SDA_HIGH();
static inline void SDA_INPUT();
static inline void SDA_OUTPUT();
static inline uint8_t SDA_READV();
static inline void SCL_OUTPUT();
static inline void SCL_LOW();
static inline void SCL_HIGH();
static inline void delay_us(uint32_t us);

void Soft_I2C_Init();
void Soft_I2C_Start();
void Soft_I2C_Restart();
void Soft_I2C_Stop();
uint8_t Soft_I2C_SendByte(uint8_t byte);
uint8_t Soft_I2C_GetByte(uint8_t recvNextByte);

uint8_t Soft_I2C_MemWrite(uint8_t devAddress, uint8_t memAddress, uint8_t *pData, uint16_t size);
uint8_t Soft_I2C_MemRead(uint8_t devAddress, uint8_t memAddress, uint8_t *pData, uint16_t size);

#endif