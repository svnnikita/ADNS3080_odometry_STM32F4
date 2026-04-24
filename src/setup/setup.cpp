#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>

#include "setup/setup.hpp"

// тактирование
void SetupPeriph::Clock_Setup() 
{
    // разгоняем мк
    rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    // тактируем линии
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOE);

    // тактирование DMA
    // rcc_periph_clock_enable(RCC_DMA1);
    // rcc_periph_clock_enable(RCC_DMA2);

    // тактирование USART2
    rcc_periph_clock_enable(RCC_USART2);

    // тактирование SPI
    rcc_periph_clock_enable(RCC_SPI1);
    // rcc_periph_clock_enable(RCC_SPI2);
    rcc_periph_clock_enable(RCC_SPI3);

    // тактирование таймера TIM6
    rcc_periph_clock_enable(RCC_TIM6);
}

// конфигурация таймера для отсчета временных задержек
void SetupPeriph::Timer_Setup() 
{
	// конфигурация вывода
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);
	timer_set_prescaler(TIM6, 168 - 1); // 168 МГц / 84 = 2 МГц
	timer_set_period(TIM6, 0xFFFF);     // 1 мкс
	timer_enable_counter(TIM6);
}

void SetupPeriph::SPI1_Setup() 
{
    // конфигурируем порты SPI1 на альтернативные функции:
    // NSS = PA4
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);

    gpio_set(GPIOA, GPIO4);

    // SCK = PA5, MISO = PA6, MOSI = PA7
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO6 | GPIO7);
    gpio_set_af(GPIOA, GPIO_AF5, GPIO5 | GPIO6 | GPIO7);

    
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5 | GPIO7);

    rcc_periph_reset_pulse(RST_SPI1);

    // настраиваем SPI1 как мастер
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_64, 
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1, 
                    SPI_CR1_DFF_8BIT, 
                    SPI_CR1_MSBFIRST);

    // управляем NSS программно
    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1);

    // включаем SPI1
    spi_enable(SPI1);
}

// SPI2 ИСПОЛЬЗУЕМ ДЛЯ ОТЛАДКИ
void SetupPeriph::SPI2_Setup() 
{
    // конфигурируем порты SPI2 на альтернативные функции:
    // NSS = PB9
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);

    // SCK = PB10, MISO = PB14, MOSI = PB15
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO14 | GPIO15);
    gpio_set_af(GPIOB, GPIO_AF5, GPIO10 | GPIO14 | GPIO15);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO10 | GPIO15);

    // перезапускаем SPI2
    rcc_periph_reset_pulse(RST_SPI2);

    // настраиваем SPI2 как мастер
    spi_init_master(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_64, 
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1, 
                    SPI_CR1_DFF_8BIT, 
                    SPI_CR1_MSBFIRST);

    // управляем NSS программно
    spi_enable_software_slave_management(SPI2);
    spi_set_nss_high(SPI2);

    // включаем SPI2
    spi_enable(SPI2);
}

void SetupPeriph::SPI3_Setup() 
{
    // конфигурируем порты SPI3 на альтернативные функции:
    // NSS = PA4
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);

    gpio_set(GPIOD, GPIO0);

    // SCK = PC10, MISO = PC11, MOSI = PC12
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOC, GPIO_AF5, GPIO10 | GPIO11 | GPIO12);

    // устанавливаем параметры вывода для SCK и MOSI
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO10 | GPIO12);

    rcc_periph_reset_pulse(RST_SPI3);

    // настраиваем SPI3 как мастер
    spi_init_master(SPI3, SPI_CR1_BAUDRATE_FPCLK_DIV_64, 
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1, 
                    SPI_CR1_DFF_8BIT, 
                    SPI_CR1_MSBFIRST);

    // управляем NSS программно
    spi_enable_software_slave_management(SPI3);
    spi_set_nss_high(SPI3);

    // включаем SPI1
    spi_enable(SPI3);
}


// конфигурируем вспомогательные выводы датчика RST и NPD
void SetupPeriph::ADNS3080PinsSetup()
{
    // правый датчик 
    // настраиваем вывод RESET и устанавливаем его в ноль
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_clear(GPIOD, GPIO1);
    // настраиваем вывод NPD для нормальной работы датчика
    gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_set(GPIOE, GPIO1);
    
    // левый датчик
    // настраиваем вывод RESET и устанавливаем его в ноль
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
    gpio_clear(GPIOB, GPIO0);
    // настраиваем вывод NPD для нормальной работы датчика
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_set(GPIOB, GPIO1);
    
}

void SetupPeriph::USART2_Setup() 
{
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