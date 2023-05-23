
#include "soft_i2c.h"

//программный i2c

void delay_us7(uint16_t us)
{
//	uint16_t curTime = TIM7->CNT;
//  while(curTime + us < TIM7->CNT)
//	{
//		__NOP();
//	}
	
	TIM7->CNT = 0;
  while(us > TIM7->CNT)
	{
		__NOP();
	}
}

void SDA_IN()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SDA_PORT, &GPIO_InitStruct);
}

void SDA_OUT()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SDA_PORT, &GPIO_InitStruct);
}

uint8_t SDA_READ()
{
	SDA_IN();
	uint8_t state = __SDA_READ();
	SDA_OUT();
	return state;
}

void SoftI2cInit()
{
	__SDA_HIGH();
	__SCL_HIGH();
}

void SoftI2cStart()
{
		SDA_OUT(); 
		__SDA_LOW(); // притянуть SDA (лог.0)
    delay_us7(10);
    __SCL_LOW(); // притянуть SCL (лог.0)
    delay_us7(10);
}

void SoftI2cStop()
{
		__SCL_LOW(); // притянуть SCL (лог.0)
    delay_us7(10);
    __SDA_LOW(); // притянуть SDA (лог.0)
    delay_us7(10);

    __SCL_HIGH(); // отпустить SCL (лог.1)
    delay_us7(10);
    __SDA_HIGH(); // отпустить SDA (лог.1)
    delay_us7(10);	
}

void SoftI2cRestart()
{
		__SDA_HIGH(); // отпустить SDA (лог.1)
    delay_us7(10);
    __SCL_HIGH(); // отпустить SCL (лог.1)
    delay_us7(10);
    __SDA_LOW(); // притянуть SDA (лог.0)
    delay_us7(10);
    __SCL_LOW(); // притянуть SCL (лог.0)
    delay_us7(10);
}

uint8_t SoftI2cSendByte(uint8_t data)
{
	uint8_t i;
	uint8_t ack=1;           //АСК, если АСК=1 – произошла ошибка
	uint16_t SDA;   
	for (i=0;i<8;i++)
	{
			if (data & 0x80) 
			{
				__SDA_HIGH(); // лог.1
			}
			else 
			{
				__SDA_LOW(); // Выставить бит на SDA (лог.0
			}
			delay_us7(10);
			__SCL_HIGH();   // Записать его импульсом на SCL       // отпустить SCL (лог.1)
			delay_us7(10);
			__SCL_LOW(); // притянуть SCL (лог.0)
			data<<=1; // сдвигаем на 1 бит влево
				
	}
	__SDA_HIGH(); // отпустить SDA (лог.1), чтобы ведомое устройство смогло сгенерировать ACK
	delay_us7(10);
	__SCL_HIGH(); // отпустить SCL (лог.1), чтобы ведомое устройство передало ACK
	delay_us7(10);
	SDA=SDA_READ();
	if (SDA==0x00) ack=1; else ack=0;    // Считать ACK

	__SCL_LOW(); // притянуть SCL (лог.0)  // приём ACK завершён

	return ack; // вернуть ACK (0) или NACK (1)  
}

uint8_t SoftI2cGetByte (uint8_t last_byte)
{
	uint8_t i, res=0;
	uint16_t SDA;
	__SDA_HIGH(); // отпустить SDA (лог.1)

	for (i=0;i<8;i++)
	{
			res<<=1;
			__SCL_HIGH(); // отпустить SCL (лог.1)      //Импульс на SCL
			delay_us7(10);
			__SDA_HIGH();
			SDA = SDA_READ();
			if (SDA==1) res=res|0x01; // Чтение SDA в переменную  Если SDA=1 то записываем 1
			__SCL_LOW(); // притянуть SCL (лог.0)
			delay_us7(10);
	}

	if (last_byte==0){ __SDA_LOW();} // притянуть SDA (лог.0)     // Подтверждение, ACK, будем считывать ещё один байт
	else {__SDA_HIGH();} // отпустить SDA (лог.1)                 // Без подтверждения, NACK, это последний считанный байт
	delay_us7(10);
	__SCL_HIGH(); // отпустить SCL (лог.1)
	delay_us7(10);
	__SCL_LOW(); // притянуть SCL (лог.0)
	delay_us7(10);
	__SDA_HIGH(); // отпустить SDA (лог.1)

	return res; // вернуть считанное значение
}
