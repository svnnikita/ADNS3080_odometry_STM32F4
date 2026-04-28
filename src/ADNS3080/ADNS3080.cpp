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


void ADNS3080::setup(const bool led_mode, const bool resolution) 
{
	// конфигурируем датчик:
	//                          LED Shutter     High resolution
	uint8_t mask = 0b00000000 | led_mode << 6 | resolution << 4;

	// записываем конфигурацию в датчик
    writeRegister(ADNS3080_CONFIGURATION_BITS, mask);
}

bool ADNS3080::checkConfiguration(const bool led_mode, const bool resolution)
{
	// маска для конфигурации
	uint8_t mask = (led_mode   ? (1 << 6) : 0) |
                    (resolution ? (1 << 4) : 0);

	volatile uint8_t config1 = readRegister(ADNS3080_CONFIGURATION_BITS);
	uint8_t config = readRegister(ADNS3080_CONFIGURATION_BITS);

	if (config == mask)
		return true;
	else 
		return false;
}

void ADNS3080::loadSROM(const uint8_t *data, uint16_t length) {
    // Шаг 1: аппаратный сброс
    reset();

    // Шаг 2-4: инициализация перед загрузкой
    writeRegister(0x20, 0x44);
    writeRegister(0x24, 0x07);
    writeRegister(0x24, 0x88);

    // Шаг 5: минимум 1 кадровый период (при 2000 fps ~ 500 мкс, берём 1 мс)
    delay_us(1000);

    // Шаг 6: включение режима загрузки SROM
    writeRegister(0x14, 0x18);  // SROM_Enable

    // Шаг 7: burst-запись массива в регистр SROM_Load (0x60)
    csLow();

    // Отправляем адрес регистра SROM_Load с битом записи (0x60 | 0x80 = 0xE0)
    spi_send(_spi, 0x60 | 0x80);
    while (!(SPI_SR(_spi) & SPI_SR_TXE));

    // Отправляем первый байт
    spi_send(_spi, data[0]);
    while (!(SPI_SR(_spi) & SPI_SR_TXE));
    delay_us(ADNS3080_T_LOAD);  // 10 мкс

    // Отправляем остальные байты с задержкой t_LOAD между ними
    for (uint16_t i = 1; i < length; i++) {
        spi_send(_spi, data[i]);
        while (!(SPI_SR(_spi) & SPI_SR_TXE));
        delay_us(ADNS3080_T_LOAD);
    }

    // Дожидаемся завершения передачи на шине
    while (SPI_SR(_spi) & SPI_SR_BSY);

    // Шаг 8: выход из burst-режима, CS поднять на t_BEXIT
    csHigh();
    delay_us(ADNS3080_T_BEXIT);  // 4 мкс
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

	while (SPI_SR(_spi) & SPI_SR_RXNE) {
		(void)SPI_DR(_spi);   // чтение регистра данных для сброса RXNE
    }

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

	while (SPI_SR(_spi) & SPI_SR_RXNE) {
        (void)SPI_DR(_spi);
    }

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
	while((pixel & 0b01000000) == 0) {
		
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