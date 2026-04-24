#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

#include <cstdio>	

#include "ADNS3080.hpp"

// конструктор
ADNS3080::ADNS3080(const Adns3080Pins &pins) 
	: _cs_gpio_port(pins.cs_gpio_port), 
	  _cs_gpio_pin(pins.cs_gpio_pin),
	  _spi(pins.spi),
	  _reset_gpio_port(pins.reset_gpio_port),
	  _reset_gpio_pin(pins.reset_gpio_pin)
{
	// отправим сообщение для отладки
    // usart_send_blocking(USART2, 'A');
}


// конфигурация датчика
bool ADNS3080::setup(const bool led_mode, const bool resolution) 
{
	// перезапускаем датчик
	reset();
	
	// конфигурируем датчик:
	//                          LED Shutter     High resolution
	uint8_t mask = 0b00000000 | led_mode << 6 | resolution << 4;     
	writeRegister(ADNS3080_CONFIGURATION_BITS, mask);


    if (readRegister(ADNS3080_CONFIGURATION_BITS) == mask)
    	return true;
    else 
        return false;
}

// задержка в микросекундах
void ADNS3080::delay_us(uint16_t delay) {
	// ждем реакции датчика
	uint32_t start = timer_get_counter(TIM6);
    while ((uint16_t)(timer_get_counter(TIM6) - start) < delay) {
		__asm__("NOP");
	}
}

// читаем регистры
uint8_t ADNS3080::readRegister(const uint8_t reg)
{
	// переменная для значения из указанного регистра
	uint8_t data;

    csLow();

	// отправляем адрес регистра
    spi_send(_spi, reg);
	// ждём конец передачи адреса
    while (!(SPI_SR(_spi) & SPI_SR_TXE));

    // в зависимости от адреса регистра устанавливаем задержку
	// для регистров проверки движения и запуска пакетного режима
	// устанавливается задержка 75 мкс
    if (reg == ADNS3080_MOTION || reg == ADNS3080_MOTION_BURST)
        delay_us(ADNS3080_T_SRAD_MOT);
    else
		// для остальных регистров задержка 50 мкс
        delay_us(ADNS3080_T_SRAD);

	// отправляем dummy‑байт, генерируем тактовые импульсы 
	// и одновременно получаем данные из регистра
    spi_send(_spi, 0x00);    
	// дожидаемся отправки dummy‑байта
    while (!(SPI_SR(_spi) & SPI_SR_TXE));
	// ждем, пока придут все данные
    while (!(SPI_SR(_spi) & SPI_SR_RXNE));
	// читаем принятые данные
    data = spi_read(_spi);

	// ждем, пока освободится шина
    while (SPI_SR(_spi) & SPI_SR_BSY);

    csHigh();

	delay_us(ADNS3080_T_SWW);

    return data;
}

// записываем значения в регистры
void ADNS3080::writeRegister(const uint8_t reg, uint8_t output) 
{
  	csLow();
  	// ждём готовности ведомого устройства
    while (!(SPI_SR(_spi) & SPI_SR_TXE));

	// устанавливаем бит 7 регистра в единицу для записи
    spi_send(_spi, reg | 0x80);          
    // ждём окончания отправки адреса
    while (!(SPI_SR(_spi) & SPI_SR_TXE));

	// отправляем данные
    spi_send(_spi, output);              
    // ждём окончания отправки данных
    while (!(SPI_SR(_spi) & SPI_SR_TXE));
    // ждём освобождения шины
    while (SPI_SR(_spi) & SPI_SR_BSY);

    csHigh();

    delay_us(ADNS3080_T_SWW);            
}


// отправляет команду датчику на отправку данных о
// качестве поверхности, движении и т.д.
void ADNS3080::motionBurst(uint8_t *motion, int8_t *dx, int8_t *dy, 
                     uint8_t *squal, uint16_t *shutter, uint8_t *max_pix)
{

}


// восстанавливаем пиксели следующего кадра:
void ADNS3080::frameCapture(uint8_t output[ADNS3080_PIXELS][ADNS3080_PIXELS]) 
{  
	// первый пиксель:
	uint8_t pixel = 0;

	// отправляем значение в регистр
	writeRegister(ADNS3080_FRAME_CAPTURE, 0x83);
	
	// опускаем линию и начинаем получение данных
	csLow();

	// получаем пиксели до тех пор, пока не будет найден первый
	while((pixel & 0B01000000) == 0) {
		
		// отправляем любой бит для получения данных из указанного регистра 
		spi_send(_spi, 0x00);	
		while (!(SPI_SR(_spi) & SPI_SR_RXNE));

		// получаем пиксель
		pixel = spi_read(_spi);
		
		delay_us(ADNS3080_T_LOAD);  
	}
	
	// анализируем первый кадр:
	for(int y = 0; y < ADNS3080_PIXELS; y++) {
		for(int x = 0; x < ADNS3080_PIXELS; x++) {  
		
			// сохраняем и масштабируем полученный кадр
			output[x][y] = pixel << 2; 

			// получаем следующий пиксель
			spi_send(_spi, 0x00);
			while (!(SPI_SR(_spi) & SPI_SR_RXNE));

			pixel = spi_read(_spi);
			delay_us(ADNS3080_T_LOAD);  
		}
	}

	// отключаем линию
	csHigh();

	// ждем реакцию датчика
	delay_us(ADNS3080_T_LOAD + ADNS3080_T_BEXIT);
}   	