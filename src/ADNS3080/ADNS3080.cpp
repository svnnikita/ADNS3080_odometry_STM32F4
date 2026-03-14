#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>

#include <cstdio>

#include "ADNS3080.hpp"

// Задержка в микросекундах
void ADNS3080::delay_us( uint16_t delay ) {
	// ждем реакции датчика
	uint16_t start = timer_get_counter(TIM6);
    while ((uint16_t)(timer_get_counter(TIM6) - start) < delay) {
		__asm__("NOP");
	}
}

// Включение/выключение датчика

// Запись в регистры
void ADNS3080::writeRegister( const uint8_t reg, uint8_t output ) {
  	// устанавливаем низкий уровень для общения с датчиком
  	gpio_clear(GPIOA, GPIO4);
  	// устанавливаем младший бит адреса регистра в единицу и отправляем данные
	spi_send(SPI1, reg | 0x80);
	// ждем, пока отправятся данные	(пока бит TXE регистра SPI_SR не установлен)
	while (!(SPI_SR(SPI1) & SPI_SR_TXE));
	spi_send(SPI1, output);
	while (!(SPI_SR(SPI1) & SPI_SR_TXE));
    // ждем, пока освободится шина
	while (SPI_SR(SPI1) & SPI_SR_BSY);	
	// восстанавливаем высокий уровень для прекращения связи с датчиком
	gpio_set(GPIOA, GPIO4);
	// ждем реакции датчика
	delay_us(ADNS3080_T_SWW);
}

// Чтение регистров
uint8_t ADNS3080::readRegister( const uint8_t reg ) {
	uint8_t output;	// полезные данные
    uint8_t dummy;	// случайные данные
	// устанавливаем низкий уровень для общения с датчиком
  	gpio_clear(GPIOA, GPIO4);
	// отправляем адрес регистра (младший бит в нуле -- режим чтения)
	spi_send(SPI1, reg);
	while (!(SPI_SR(SPI1) & SPI_SR_RXNE));
	dummy = spi_read(SPI1);
	// ждем реакции датчика
	delay_us(ADNS3080_T_SRAD_MOT);
	// отправляем любой бит для получения данных из указанного регистра 
	spi_send(SPI1, 0x00);
	while (!(SPI_SR(SPI1) & SPI_SR_RXNE));
	// получаем заветный бит
	output = spi_read(SPI1);
	// ждем, пока освободится шина
	while (SPI_SR(SPI1) & SPI_SR_BSY);
	// отключаем линию
	gpio_set(GPIOA, GPIO4);
	return output;
}


// Перезагрузка датчика
void ADNS3080::reset() {
	// подаем на вывод перезагрузки высокий сигнал
	gpio_set(GPIOB, GPIO0);
	// ждем реакции датчика
	delay_us(ADNS3080_T_PW_RESET);
	// опускаем сигнал
	gpio_clear(GPIOB, GPIO0);
	// ждем реакции датчика
	delay_us(50000);      
}


// Конфигурация датчика
bool ADNS3080::setup( const bool led_mode, const bool resolution ) {
	// опускаем линию SPI1_NC и перезагружаем датчик
	gpio_set(GPIOA, GPIO4);
	reset();
	// конфигурируем датчик:
	//                           LED Shutter    High resolution
	uint8_t mask = 0b00000000 | led_mode << 6 | resolution << 4;     
	writeRegister( ADNS3080_CONFIGURATION_BITS, mask );
	// проверяем подключение
	// if( readRegister(ADNS3080_PRODUCT_ID) == ADNS3080_PRODUCT_ID_VALUE ) return true;
	// else return false;
	uint8_t id = readRegister(ADNS3080_PRODUCT_ID);
	char buf[32];
	snprintf(buf, sizeof(buf), "Read ID: 0x%02X\r\n", id);
	for (int i = 0; buf[i]; i++) {
        usart_send_blocking(USART2, buf[i]);
    }
    
    return (id == ADNS3080_PRODUCT_ID_VALUE);
}

// Очистка регистров перемещения, DELTA_X, DELTA_Y
void ADNS3080::motionClear() {
	writeRegister( ADNS3080_MOTION_CLEAR, 0x00 );	
}

//Получение пакета данных о перемещении
void ADNS3080::motionBurst(uint8_t *buffer) {
	uint8_t dummy;							    // случайные данные
	// для запроса данных нужно отправить любое значение в регистр ADNS3080_MOTION_BURST
	// затем датчик начнет отправлять последовательно значения регистров Motion, Delta_X, 
	// Delta_Y, SQUAL, Shutter_Upper, Shutter_Lower и Maximum_Pixel
	// полученные 56 байт будут отправлены на компьютер и разбиты на вышеуказанные значения
	// опускаем линию и начинаем получение данных
	gpio_clear(GPIOA, GPIO4);
	// отправляем адрес регистра (младший бит в нуле -- режим чтения)
	spi_send(SPI1, ADNS3080_MOTION_BURST);
	while (!(SPI_SR(SPI1) & SPI_SR_RXNE));
	// получаем случайные данные
	dummy = spi_read(SPI1);
	// ждем реакции датчика
	delay_us(ADNS3080_T_SRAD_MOT);
	// получаем полезные данные
	for (uint8_t i = 0; i < MOT_BURST_ALL_DATA; i++) {
		// отправляем любой бит для получения данных из указанного регистра 
		spi_send(SPI1, 0x00);
		while (!(SPI_SR(SPI1) & SPI_SR_RXNE));
		buffer[i] = spi_read(SPI1);
	}
	// ждем, пока освободится шина
	while (SPI_SR(SPI1) & SPI_SR_BSY);
	// отключаем линию
	gpio_set(GPIOA, GPIO4);
}


// Получение данных о перемещении из пакета данных о движении
// void ADNS3080::displacement() {
// 	uint8_t displacement_bytes[MOT_BURST_DISPL_DATA];	// полезные данные
// 	uint8_t dummy;

// 	// опускаем линию и начинаем получение данных
// 	gpio_clear(GPIOA, GPIO4);

// 	// отправляем адрес регистра (младший бит в нуле -- режим чтения)
// 	spi_send(SPI1, ADNS3080_MOTION_BURST);
// 	while (!(SPI_SR(SPI1) & SPI_SR_RXNE));

// 	// получаем случайные данные
// 	dummy = spi_read(SPI1);

// 	// ждем реакции датчика
// 	delay_us(ADNS3080_T_SRAD_MOT);
// }


// получаем сырые пиксели с матрицы датчика:
void ADNS3080::frameCapture( uint8_t output[ADNS3080_PIXELS][ADNS3080_PIXELS] ) {  
	uint8_t dummy;	// случайные данные

	// отправляем значение в регистр
	// это необходимо для считывания сырых пикселей с матрицы датчика
	writeRegister(ADNS3080_FRAME_CAPTURE, 0x83);
	
	// опускаем линию и начинаем получение данных
	gpio_clear(GPIOA, GPIO4);

	// отправляем адрес регистра (младший бит в нуле -- режим чтения)
	spi_send(SPI1, ADNS3080_PIXEL_BURST);
	while (!(SPI_SR(SPI1) & SPI_SR_RXNE));

	// читаем случайные данные
	dummy = spi_read(SPI1);
	delay_us(ADNS3080_T_SRAD);

	// первый пиксель:
	uint8_t pixel = 0;

	// ищем значение первого пикселя
	while((pixel & 0B01000000) == 0) {
		
		// отправляем любой бит для получения данных из указанного регистра 
		spi_send(SPI1, 0x00);	
		while (!(SPI_SR(SPI1) & SPI_SR_RXNE));

		// получаем пиксель
		pixel = spi_read(SPI1);
		delay_us(ADNS3080_T_LOAD);  
	}
	
	// идем по массиву и записываем полученные пиксели:
	for ( uint8_t i = 0; i < ADNS3080_PIXELS; i++ ) {
		for ( uint8_t j = 0; j < ADNS3080_PIXELS; j++ ) {  
		
			// до масштабирования данные представляют собой значения от 0 до 63
			// масштабируем кадр, чтобы получить данные в диапазоне 0 - 255
			output[i][j] = pixel << 2; 

			// получаем следующий пиксель
			spi_send(SPI1, 0x00);
			while (!(SPI_SR(SPI1) & SPI_SR_RXNE));

			pixel = spi_read(SPI1);
			delay_us(ADNS3080_T_LOAD);  
		}
	}

	// отключаем линию
	gpio_set(GPIOA, GPIO4);
	delay_us(ADNS3080_T_LOAD + ADNS3080_T_BEXIT);
}