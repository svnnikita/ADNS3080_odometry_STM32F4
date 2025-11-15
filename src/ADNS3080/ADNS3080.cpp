#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/timer.h>

#include "ADNS3080.hpp"

// Задержка в микросекундах
void ADNS3080::delay_us(uint16_t delay) {
	// ждем реакции датчика
	uint16_t start = timer_get_counter(TIM6);
    while ((uint16_t)(timer_get_counter(TIM6) - start) < delay) {
		__asm__("NOP");
	}
}

// Запись в регистры
void ADNS3080::writeRegister( const uint8_t reg, uint8_t output ) {
  	// устанавливаем низкий уровень для общения с датчиком
  	gpio_clear(GPIOB, GPIO9);

  	// устанавливаем младший бит адреса регистра в единицу и отправляем данные
	spi_send(SPI2, reg | 0x80);

	// ждем, пока отправятся данные	(пока бит TXE регистра SPI_SR не установлен)
	while (!(SPI_SR(SPI2) & SPI_SR_TXE))
			;

	spi_send(SPI2, output);
	while (!(SPI_SR(SPI2) & SPI_SR_TXE))
			;

    // ждем, пока освободится шина
	while (SPI_SR(SPI2) & SPI_SR_BSY)
			;
			
	// восстанавливаем высокий уровень для прекращения связи с датчиком
	gpio_set(GPIOB, GPIO9);

	// ждем реакции датчика
	delay_us(ADNS3080_T_SWW);
}

// Чтение регистров
uint8_t ADNS3080::readRegister( const uint8_t reg ) {
	uint8_t output;
    uint16_t dummy;

	// устанавливаем низкий уровень для общения с датчиком
  	gpio_clear(GPIOB, GPIO9);

	// отправляем адрес регистра (младший бит в нуле -- режим чтения)
	spi_send(SPI2, reg);
	while (!(SPI_SR(SPI2) & SPI_SR_RXNE))
		;

	dummy = spi_read(SPI2);

	// ждем реакции датчика
	delay_us(ADNS3080_T_SRAD_MOT);

	// отправляем любой бит для получения данных из указанного регистра 
	spi_send(SPI2, 0x00);
	while (!(SPI_SR(SPI2) & SPI_SR_RXNE))
			;

	// получаем заветный бит
	output = spi_read(SPI2);

	// ждем, пока освободится шина
	while (SPI_SR(SPI2) & SPI_SR_BSY);
	
	// отключаем линию
	gpio_set(GPIOB, GPIO9);

	return output;
}

// Перезагрузка датчика
void ADNS3080::reset() {
	// подаем на вывод перезагрузки высокий сигнал
	gpio_set(GPIOA, GPIO10);

	// ждем реакции датчика
	delay_us(ADNS3080_T_PW_RESET);

	// опускаем сигнал
	gpio_clear(GPIOA, GPIO10);

	// ждем реакции датчика
	delay_us(ADNS3080_T_IN_RST);      
}

// Конфигурация датчика
bool ADNS3080::setup( const bool led_mode, const bool resolution ) {

	// отключаем датчик и перезагружаем
	gpio_set(GPIOB, GPIO9);
	reset();
	
	// конфигурируем датчик:
	//                           LED Shutter    High resolution
	uint8_t mask = 0b00000000 | led_mode << 6 | resolution << 4;     
	writeRegister( ADNS3080_CONFIGURATION_BITS, mask );

	// проверяем подключение
	if( readRegister(ADNS3080_PRODUCT_ID) == ADNS3080_PRODUCT_ID_VALUE ) return true;
		else return false;
}

// восстановливаем пиксели следующего кадра:
void ADNS3080::frameCapture( uint8_t output[ADNS3080_PIXELS][ADNS3080_PIXELS] ) {  
	// переменная для приема мусорных данных
	uint8_t dummy;

	// отправляем значение в регистр
	writeRegister(ADNS3080_FRAME_CAPTURE, 0x83);
	
	// опускаем линию и начинаем получение данных
	gpio_clear(GPIOB, GPIO9);

	// отправляем адрес регистра (младший бит в нуле -- режим чтения)
	spi_send(SPI2, ADNS3080_PIXEL_BURST);
	while (!(SPI_SR(SPI2) & SPI_SR_RXNE))
		;

	dummy = spi_read(SPI2);

	delay_us(ADNS3080_T_SRAD);

	//-- первый пиксель:
	uint8_t pixel = 0;

	// получаем пиксели до тех пор, пока не будет найден первый
	while((pixel & 0B01000000) == 0) {
		
		// отправляем любой бит для получения данных из указанного регистра 
		spi_send(SPI2, 0x00);	
		while (!(SPI_SR(SPI2) & SPI_SR_RXNE))
			;

		// получаем пиксель
		pixel = spi_read(SPI2);
		
		delay_us(ADNS3080_T_LOAD);  
	}
	
	//-- анализируем первый кадр:
	for( int y = 0; y < ADNS3080_PIXELS; y++ ) {
		for( int x = 0; x < ADNS3080_PIXELS; x++ ) {  
		
			// сохраняем и масштабируем полученный кадр
			output[x][y] = pixel << 2; 

			// получаем следующий пиксель
			spi_send(SPI2, 0x00);
			while (!(SPI_SR(SPI2) & SPI_SR_RXNE))
				;

			pixel = spi_read(SPI2);
			delay_us(ADNS3080_T_LOAD);  
		}
	}

	// отключаем линию
	gpio_set(GPIOB, GPIO9);
	delay_us(ADNS3080_T_LOAD + ADNS3080_T_BEXIT);
}   	