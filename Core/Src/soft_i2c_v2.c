
#include "soft_i2c_v2.h"

static inline void SDA_LOW()
{
	//выставить логический 0 на SDA
	
  PORT_SDA->BSRR |= (1 << (PIN_SDA + 0x10));
}
static inline void SDA_HIGH()
{
	//выставить логическую 1 на SDA
	
	PORT_SDA->BSRR |= (1 << PIN_SDA);
}
static inline void SCL_LOW()
{
	//выставить логический 0 на SCL
	
	PORT_SCL->BSRR |= (1 << (PIN_SCL + 0x10));
}

static inline void SCL_HIGH()
{
	//выставить логическую 1 на SCL
	
	PORT_SCL->BSRR |= (1 << PIN_SCL);
}

static inline void delay_us(uint32_t us)
{
	//организация микросекундной задержки
	
	TIM7->CNT = 0;
  while(us > TIM7->CNT)
	{
		__NOP();
	}
}

static inline void SDA_OUTPUT()
{
	//сконфигурировать ножку SDA на выход
	
	uint16_t buf = PORT_SDA->MODER;
	buf |= 1 << (PIN_SDA * 2);
	buf &= ~(1 << (PIN_SDA * 2 + 1));
	PORT_SDA->MODER = buf;
}

static inline void SCL_OUTPUT()
{
	//сконфигурировать ножку SCL на выход
	
  uint16_t buf = PORT_SCL->MODER;
	buf |= 1 << (PIN_SCL * 2);
	buf &= ~(1 << (PIN_SCL * 2 + 1));
	PORT_SCL->MODER = buf;
}

static inline void SDA_INPUT()
{
	//сконфигурировать ножку SDA на вход
	
  uint16_t buf = PORT_SDA->MODER;
	buf &= ~(1 << (PIN_SDA * 2));
	buf &= ~(1 << (PIN_SDA * 2 + 1));
	PORT_SDA->MODER = buf;
}

static inline uint8_t SDA_READV()
{
	// чтение состояни пина SDA
	
	uint8_t reg = PORT_SDA->IDR;
	reg &= (1 << PIN_SDA);
	reg = reg >> PIN_SDA;
	return reg;
}

void Soft_I2C_Init()
{
	SDA_HIGH();
	SCL_HIGH();
	SDA_OUTPUT();
	SCL_OUTPUT();
}

void Soft_I2C_Start()
{
	SDA_LOW();
	delay_us(DELAY_US);
	SCL_LOW();
	delay_us(DELAY_US);
}

void Soft_I2C_Stop()
{
	SCL_LOW(); // притянуть SCL (лог.0)
	delay_us(DELAY_US);
	SDA_LOW(); // притянуть SDA (лог.0)
	delay_us(DELAY_US);

	SCL_HIGH(); // отпустить SCL (лог.1)
	delay_us(DELAY_US);
	SDA_HIGH(); // отпустить SDA (лог.1)
	delay_us(DELAY_US); 		
}

void Soft_I2C_Restart(void)   // функция генерации условия рестарт
{
	SDA_HIGH(); // отпустить SDA (лог.1)
	delay_us(DELAY_US);
	SCL_HIGH(); // отпустить SCL (лог.1)
	delay_us(DELAY_US);
	SDA_LOW(); // притянуть SDA (лог.0)
	delay_us(DELAY_US);
	SCL_LOW(); // притянуть SCL (лог.0)
	delay_us(DELAY_US);
}

uint8_t Soft_I2C_SendByte(uint8_t byte)
{
	uint8_t ack;
	SCL_LOW();
	
	for(uint8_t i = 0; i < 8; i++)
	{
		if(byte & 0x80)
		{
			SDA_HIGH();
		}
		else
		{
			SDA_LOW();
		}
		
		delay_us(DELAY_US);
		SCL_HIGH();
		delay_us(DELAY_US);
		SCL_LOW();
		
		byte <<= 1;
	}
	SDA_INPUT();
	delay_us(DELAY_US);
	SCL_HIGH();
	delay_us(DELAY_US);
	ack = SDA_READV();
	SCL_LOW();
	delay_us(DELAY_US);
	
	SDA_OUTPUT();
	SDA_LOW(); 
	
	return ack;
}

uint8_t Soft_I2C_GetByte(uint8_t recvNextByte)
{
	uint8_t res=0;
	uint8_t sda;
	//SDA_HIGH(); // отпустить SDA (лог.1)

	for (uint8_t i=0;i<8;i++)
	{
			res<<=1;
			SCL_HIGH(); // отпустить SCL (лог.1)      //Импульс на SCL
			//SDA_INPUT();
			delay_us(DELAY_US);
			SDA_INPUT(); //проверить!!
			SDA_HIGH();
			sda = SDA_READV();
			if (sda==1) res=res|0x01; // Чтение SDA в переменную  Если SDA=1 то записываем 1
			SCL_LOW(); // притянуть SCL (лог.0)
			delay_us(DELAY_US);
			SDA_INPUT();
	}

	if (recvNextByte==0){ SDA_LOW();} // притянуть SDA (лог.0)     // Подтверждение, ACK, будем считывать ещё один байт
	else {SDA_HIGH();} // отпустить SDA (лог.1)                 // Без подтверждения, NACK, это последний считанный байт
	SDA_OUTPUT();
	delay_us(DELAY_US);
	SCL_HIGH(); // отпустить SCL (лог.1)
	delay_us(DELAY_US);
	SCL_LOW(); // притянуть SCL (лог.0)
	delay_us(DELAY_US);
	SDA_HIGH(); // отпустить SDA (лог.1)
  //delay_us(DELAY_US);
	
	return res; // вернуть считанное значение
}

uint8_t Soft_I2C_MemWrite(uint8_t devAddress, uint8_t memAddress, uint8_t *pData, uint16_t size)
{
	uint8_t rezultOperation = 0, ack;
	
	Soft_I2C_Start();
	
	ack = Soft_I2C_SendByte(devAddress << 1);//отправка 7-битного адреса устройства
	if(ack == 1) {rezultOperation = ack; return rezultOperation;}
	
	ack = Soft_I2C_SendByte(memAddress);		//отправка 8-битного адреса регистра в устройстве
	if(ack == 1) {rezultOperation = ack; return rezultOperation;}
	
	for(uint16_t i = 0; i < size; i++) //отправка данных
	{
		ack = Soft_I2C_SendByte(pData[i]);
		if(ack == 1) {rezultOperation = ack; return rezultOperation;}
	}
	
	Soft_I2C_Stop();
	
	return rezultOperation;
}

uint8_t Soft_I2C_MemRead(uint8_t devAddress, uint8_t memAddress, uint8_t *pData, uint16_t size)
{
	uint8_t rezultOperation = 0, ack;
	
	Soft_I2C_Start();
	
	ack = Soft_I2C_SendByte(devAddress << 1); //отправка 7-битного адреса ус-ва с режимом записи
	if(ack == 1) {rezultOperation = ack; return rezultOperation;}
	
	ack = Soft_I2C_SendByte(memAddress);
	if(ack == 1) {rezultOperation = ack; return rezultOperation;}
	
	Soft_I2C_Restart();
	
	ack = Soft_I2C_SendByte((devAddress << 1) + 1); //отправка 7-битного адреса с режимом чтения
	if(ack == 1) {rezultOperation = ack; return rezultOperation;}
	
	for(uint16_t i = 0; i < size; i++)
	{
		if(i == size - 1)
		{
			pData[i] = Soft_I2C_GetByte(1);
		}
		else
			pData[i] = Soft_I2C_GetByte(0);
	}
	
	Soft_I2C_Stop();
	
	return rezultOperation;
}
