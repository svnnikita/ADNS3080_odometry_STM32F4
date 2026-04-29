#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/ethernet/mac.h>
#include <libopencm3/ethernet/phy.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>

#include "setup/setup.hpp"

// тактирование
void SetupPeriph::clockSetup() 
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

    // тактирование таймера TIM6
    rcc_periph_clock_enable(RCC_TIM6);

    // тактирование SPI
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_SPI3);
    
    // тактирование USART2
    rcc_periph_clock_enable(RCC_USART2);
}

// конфигурация таймера для отсчета временных задержек
void SetupPeriph::timerSetup() 
{
	// конфигурация вывода
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);
    
    // rcc_apb1_frequency = 42 МГц
	timer_set_prescaler(TIM6, (rcc_apb1_frequency * 2) - 1);  // 84 МГц / 84 = 2 МГц
	timer_set_period(TIM6, 0xFFFF);     // 1 мкс
	timer_enable_counter(TIM6);
}

// задержка в микросекундах
void SetupPeriph::delayUs(uint16_t delay) {
	// ждем реакции датчика
	uint32_t start = timer_get_counter(TIM6);
    while ((uint16_t)(timer_get_counter(TIM6) - start) < delay) {
		__asm__("NOP");
	}
}

// конфигурация портов MAC контроллера
void SetupPeriph::macSetup()
{
    // RESET
    gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);
    // сбрасываем состояние phy
    gpio_clear(GPIOE, GPIO15);

    // удерживаем сигнал nRST 400 мкс в состоянии 0
    delayUs(T_nRSTIA);
    // поднимаем сигнал перезагрузки
    gpio_set(GPIOE, GPIO15);

    // ждем 5 мс (5000 мкс) для доступа к PHY по SMI
    delayUs(T_AFTERnRST);

    // тактируем контроллер конфигурации системы
    // (используется для выбора Ethernet PHY)
    rcc_periph_clock_enable(RCC_SYSCFG);

    // до тактирования MAC устанавливаем режим RMII
    // для этого выставляем бит 23 MII_RMII_SEL в единицу
    SYSCFG_PMC |= (1 << 23);

    // тактируем модуль MAC
    rcc_periph_clock_enable(RCC_ETHMAC);
    rcc_periph_clock_enable(RCC_ETHMACTX);
    rcc_periph_clock_enable(RCC_ETHMACRX);

    // REF_CLK, MDIO, CRS_DV
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO1 | GPIO2 | GPIO7);
    gpio_set_af(GPIOA, GPIO_AF11, GPIO1 | GPIO2 | GPIO7);

    // TXEN, TXD0 и TXD1
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12 | GPIO13);
    gpio_set_af(GPIOB, GPIO_AF11, GPIO11 | GPIO12 | GPIO13);

    // MDC, RXD0, RXD1
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO1 | GPIO4 | GPIO5);
    gpio_set_af(GPIOC, GPIO_AF11, GPIO1 | GPIO4 | GPIO5);

    // инициализируем ethernet, настраиваем тактирование и инициализируем DMA
    eth_init(0, ETH_CLK_150_168MHZ);
}

// SPI1 -- левая камера
void SetupPeriph::spi1Setup() 
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
void SetupPeriph::spi3Setup() 
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
void SetupPeriph::adns3080PinsSetup()
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

// конфигурация USART2
void SetupPeriph::usart2Setup() 
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


