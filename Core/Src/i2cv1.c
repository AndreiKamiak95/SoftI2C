#include "i2cv1.h"

volatile uint8_t i2c_frame_error=0; 

//-----------------------------------------------------------
__STATIC_INLINE void Delay_us (uint32_t __IO us) //Функция задержки в микросекундах us
{
us *=(SystemCoreClock/1000000)/5;
	while(us--);
}

//----------------------------------------------------
void SCL_in (void) //функция отпускания SCL в 1, порт на вход (необходимо установить используемый порт) 
	{
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.Pin = GPIO_PIN_6;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
//----------------------------------------------------
void SCL_out (void) //функция притягивания SCL в 0 (необходимо установить используемый порт) 
	{
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.Pin = GPIO_PIN_6;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	SCL_O;
	}
//----------------------------------------------------
void SDA_in (void) //функция отпускания SDA в 1, порт на вход (необходимо установить используемый порт) 
	{
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.Pin = GPIO_PIN_7;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
//----------------------------------------------------
void SDA_out (void) //функция притягивания SDA в 0 (необходимо установить используемый порт) 
	{
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.Pin = GPIO_PIN_7;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	SDA_O
	}

//----------------------------------------------------
void i2c_stop_cond (void)  // функция генерации условия стоп 
{
    uint16_t SCL, SDA;
		SCL_out(); // притянуть SCL (лог.0)
    Delay_us(10);
    SDA_out(); // притянуть SDA (лог.0)
    Delay_us(10);

    SCL_in(); // отпустить SCL (лог.1)
    Delay_us(10);
    SDA_in(); // отпустить SDA (лог.1)
    Delay_us(10);
    		
    // проверка фрейм-ошибки
    i2c_frame_error=0;		// сброс счётчика фрейм-ошибок
    SCL=SCL_I;
		SDA=SDA_I;
		if (SCL == 0) i2c_frame_error++;   // проберяем, чтобы на ноге SDA была лог.1, иначе выдаём ошибку фрейма
    if (SDA == 0) i2c_frame_error++;   // проберяем, чтобы на ноге SCL была лог.1, иначе выдаём ошибку фрейма
    Delay_us(40);
   }

void i2c_init (void) // функция инициализации шины
{
    i2c_stop_cond();   // стоп шины
    i2c_stop_cond();   // стоп шины
}
//----------------------------------------------------
void i2c_start_cond (void)  // функция генерации условия старт
{
		SDA_out(); // притянуть SDA (лог.0)
    Delay_us(10);
    SCL_out(); // притянуть SCL (лог.0)
    Delay_us(10);
}
//----------------------------------------------------
void i2c_restart_cond (void)   // функция генерации условия рестарт
{
    SDA_in(); // отпустить SDA (лог.1)
    Delay_us(10);
    SCL_in(); // отпустить SCL (лог.1)
    Delay_us(10);
    SDA_out(); // притянуть SDA (лог.0)
    Delay_us(10);
    SCL_out(); // притянуть SCL (лог.0)
    Delay_us(10);
}
//----------------------------------------------------
uint8_t i2c_send_byte (uint8_t data)  // функция  отправки байта  
{   
 uint8_t i;
 uint8_t ack=1;           //АСК, если АСК=1 – произошла ошибка
uint16_t SDA;   
	for (i=0;i<8;i++)
    {
        if (data & 0x80) 
				{
				SDA_in(); // лог.1
        }
				else 
				{
				SDA_out(); // Выставить бит на SDA (лог.0
				}
        Delay_us(10);
        SCL_in();   // Записать его импульсом на SCL       // отпустить SCL (лог.1)
        Delay_us(10);
        SCL_out(); // притянуть SCL (лог.0)
        data<<=1; // сдвигаем на 1 бит влево
					
    }
    SDA_in(); // отпустить SDA (лог.1), чтобы ведомое устройство смогло сгенерировать ACK
     Delay_us(10);
    SCL_in(); // отпустить SCL (лог.1), чтобы ведомое устройство передало ACK
     Delay_us(10);
    SDA=SDA_I;
		if (SDA==0x00) ack=1; else ack=0;    // Считать ACK

    SCL_out(); // притянуть SCL (лог.0)  // приём ACK завершён

    return ack; // вернуть ACK (0) или NACK (1)   

}
//----------------------------------------------------
uint8_t i2c_get_byte (uint8_t last_byte) // функция принятия байта
{
 uint8_t i, res=0;
	uint16_t SDA;
    SDA_in(); // отпустить SDA (лог.1)

    for (i=0;i<8;i++)
    {
        res<<=1;
        SCL_in(); // отпустить SCL (лог.1)      //Импульс на SCL
        Delay_us(10);
				SDA_in();
				SDA=SDA_I;
				if (SDA==1) res=res|0x01; // Чтение SDA в переменную  Если SDA=1 то записываем 1
        SCL_out(); // притянуть SCL (лог.0)
        Delay_us(10);
    }

    if (last_byte==0){ SDA_out();} // притянуть SDA (лог.0)     // Подтверждение, ACK, будем считывать ещё один байт
    else {SDA_in();} // отпустить SDA (лог.1)                 // Без подтверждения, NACK, это последний считанный байт
    Delay_us(10);
    SCL_in(); // отпустить SCL (лог.1)
    Delay_us(10);
    SCL_out(); // притянуть SCL (лог.0)
    Delay_us(10);
    SDA_in(); // отпустить SDA (лог.1)

    return res; // вернуть считанное значение
}