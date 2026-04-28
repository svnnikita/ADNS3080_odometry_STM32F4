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
    rcc_clock_setup_pll(&rcc_hse_16mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    // тактируем линии
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOE);

    // тактирование DMA
    // rcc_periph_clock_enable(RCC_DMA1);
    // rcc_periph_clock_enable(RCC_DMA2);

    // тактируем модуль MAC
    // rcc_periph_clock_enable(RCC_ETHMAC);    // сам модуль
    // rcc_periph_clock_enable(RCC_ETHMACTX);  // линии передачи
    // rcc_periph_clock_enable(RCC_ETHMACRX);  // линии приема

    // тактирование таймера TIM6
    rcc_periph_clock_enable(RCC_TIM6);

    // тактирование SPI
    rcc_periph_clock_enable(RCC_SPI1);
    // rcc_periph_clock_enable(RCC_SPI2);
    rcc_periph_clock_enable(RCC_SPI3);
    
    // тактирование USART2
    rcc_periph_clock_enable(RCC_USART2);
}

// конфигурация таймера для отсчета временных задержек
void SetupPeriph::Timer_Setup() 
{
	// конфигурация вывода
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);
    
    // rcc_apb1_frequency = 42 МГц
	timer_set_prescaler(TIM6, (rcc_apb1_frequency * 2) - 1);  // 84 МГц / 84 = 2 МГц
	timer_set_period(TIM6, 0xFFFF);     // 1 мкс
	timer_enable_counter(TIM6);
}

// конфигурация портов MAC контроллера
void SetupPeriph::MAC_Setup()
{
    // MDIO
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
    gpio_set_af(GPIOA, GPIO_AF11, GPIO2);

    // MDC
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO1);
    gpio_set_af(GPIOC, GPIO_AF11, GPIO1);
}

// SPI1 -- левая камера
void SetupPeriph::SPI1_Setup() 
{
    // конфигурируем порты SPI1 на альтернативные функции:
    // NSS = PA4
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);

    gpio_set(GPIOA, GPIO4);

    // SCK = PA5, MISO = PA6
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO6);
    gpio_set_af(GPIOA, GPIO_AF5, GPIO5 | GPIO6);

    // MOSI = PB5
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
    gpio_set_af(GPIOB, GPIO_AF5, GPIO5);

    // устанавливаем параметры вывода для SCK и MOSI
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5);

    spi_disable(SPI1);
    // rcc_periph_reset_pulse(RST_SPI1);

    // настраиваем SPI1 как мастер
    // т.к. SPI1 тактируется от APB2 (84 МГц), то 
    // скорость SPI1 равна 84/64 = 1,3 МГц

    spi_init_master(SPI1,                              
                    // конфигурация тактового сигнала
                    // скорость
                    SPI_CR1_BAUDRATE_FPCLK_DIV_64,      

                    // полярность CPOL = 0 -- вне процесса передачи данных
                    // тактовый сигнал удерживается в нуле
                    // при этом передний фронт, по которому происходит захват
                    // данных, определяется как скачок 0-1
                    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                    
                    // фаза CPHA = 0 -- данные фиксируются по
                    // переднему фронту тактового сигнала
                    SPI_CR1_CPHA_CLK_TRANSITION_2,

                    SPI_CR1_DFF_8BIT,                   // формат кадра данных - 8 бит
                    SPI_CR1_MSBFIRST);                  // первый бит - старший

    // управляем NSS программно
    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1);

    // включаем SPI1
    spi_enable(SPI1);
}

// SPI3 -- правая камера
void SetupPeriph::SPI3_Setup() 
{
    // конфигурируем порты SPI3 на альтернативные функции:
    // NSS = PD0
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);

    gpio_set(GPIOD, GPIO0);

    // SCK = PC10, MISO = PC11, MOSI = PC12
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOC, GPIO_AF5, GPIO10 | GPIO11 | GPIO12);

    // устанавливаем параметры вывода для SCK и MOSI
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO10 | GPIO12);

    rcc_periph_reset_pulse(RST_SPI3);

    // настраиваем SPI3 как мастер
    // т.к. SPI3 тактируется от APB1 (42 МГц), то 
    // скорость SPI3 равна 42/32 = 1,3 МГц
    spi_init_master(SPI3, SPI_CR1_BAUDRATE_FPCLK_DIV_32, 
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_2, 
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
    gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
    gpio_set_af(GPIOD, GPIO_AF7, GPIO5);

    gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5);

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