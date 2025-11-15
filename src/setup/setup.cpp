#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>

#include "setup/setup.hpp"

// тактирование
void Clock_Setup() {
    // разгоняем мк
    rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    // тактирование светодиода и USART2
    rcc_periph_clock_enable(RCC_GPIOA);

    // тактирование USART2
    rcc_periph_clock_enable(RCC_USART2);

    // тактирование SPI1
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_SPI2);

    // тактирование таймера TIM6
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_TIM6);
}

// конфигурация таймера для отсчета временных задержек
void Timer_Setup() {
	// конфигурация вывода
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);
	
	timer_set_prescaler(TIM6, 168 - 1);    // 168 МГц / 84 = 2 МГц
	timer_set_period(TIM6, 0xFFFF);     // 1 мкс
	timer_enable_counter(TIM6);
}

void SPI2_Setup() {
    
    // настраиваем светодиод на PA9
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);

    // настраиваем вывод для перезагрузки датчика и устанавливаем его в ноль
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO10);
    gpio_clear(GPIOA, GPIO10);

    // конфигурируем порты SPI2 на альтернативные функции:
    // NSS = PB9
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);

    // SCK = PB10, MISO = PB14, MOSI = PB15
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO14 | GPIO15);
    gpio_set_af(GPIOB, GPIO_AF5, GPIO10 | GPIO14 | GPIO15);

    // перезапускаем spi
    rcc_periph_reset_pulse(RST_SPI2);

    // настраиваем SPI2 как мастер
    spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_64, 
                    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_2, 
                    SPI_CR1_DFF_8BIT, 
                    SPI_CR1_MSBFIRST);

    // управляем NSS программно
    spi_enable_software_slave_management(SPI2);
    spi_set_nss_high(SPI2);

    // включаем SPI2
    spi_enable(SPI2);
}

void USART2_Setup() {
    // настраиваем вывод USART2_TX
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);

    // настраиваем UART
    usart_set_baudrate(USART2, BAUD_SPEED);
    usart_set_databits(USART2, WORD_SIZE);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    // включаем UART
    usart_enable(USART2);
}