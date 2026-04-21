#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>

#include "SetupPeriph.hpp"

// тактирование
void SetupPeriph::Clock_Setup() {
    // разгоняем мк
    rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    // тактируем линии
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    // rcc_periph_clock_enable(RCC_GPIOD);
    // тактирование DMA
    // rcc_periph_clock_enable(RCC_DMA1);
    // rcc_periph_clock_enable(RCC_DMA2);
    // тактирование USART2
    rcc_periph_clock_enable(RCC_USART2);
    // тактирование SPI
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_SPI3);
    // тактирование таймера TIM6
    rcc_periph_clock_enable(RCC_TIM6);
}

// конфигурация таймера для отсчета временных задержек
void SetupPeriph::Timer_Setup() {
	// конфигурация вывода
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);
	
	timer_set_prescaler(TIM6, 168 - 1);     // 168 МГц / 84 = 2 МГц
	timer_set_period(TIM6, 0xFFFF);         // 1 мкс
	timer_enable_counter(TIM6);
}

// конфигурация DMA2 для получения данных по SPI1_RX
// прием и передача данных по SPI происходит одновременно,
// поэтому метод включает в себя конфигурацию DMA2 SPI1_TX
void SetupPeriph::DMA2_SPI1_Rx_Recv(uint8_t *zeroData, uint8_t *dataBuffer, uint16_t size) {
    // SPI1_TX, используем поток 3 и канал 3
    // dma_stream_reset(DMA2, DMA_STREAM3);
    // dma_set_peripheral_address(DMA2, DMA_STREAM3, (uint32_t)SPI1_DR);
    // dma_set_memory_address(DMA2, DMA_STREAM3, (uint32_t)zeroData);
    // dma_set_number_of_data(DMA2, DMA_STREAM3, size);
    // // ИЗ ПАМЯТИ В ПЕРИФЕРИЮ
    // dma_set_transfer_mode(DMA2, DMA_STREAM3, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    // dma_enable_memory_increment_mode(DMA2, DMA_STREAM3);
    // dma_set_peripheral_size(DMA2, DMA_STREAM3, DMA_SxCR_PSIZE_8BIT);
    // dma_set_memory_size(DMA2, DMA_STREAM3, DMA_SxCR_MSIZE_8BIT);
    // dma_set_priority(DMA2, DMA_STREAM3, DMA_SxCR_PL_MEDIUM);
    // dma_channel_select(DMA2, DMA_STREAM3, DMA_SxCR_CHSEL_3);
    // dma_enable_stream(DMA2, DMA_STREAM3);
    
    // // SPI1_RX, используем поток 0 и канал 3
    // dma_stream_reset(DMA2, DMA_STREAM0);
    // dma_set_peripheral_address(DMA2, DMA_STREAM0, (uint32_t)SPI1_DR);
    // dma_set_memory_address(DMA2, DMA_STREAM0, (uint32_t)dataBuffer);
    // dma_set_number_of_data(DMA2, DMA_STREAM0, size);
    // dma_set_transfer_mode(DMA2, DMA_STREAM0, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    // dma_enable_memory_increment_mode(DMA2, DMA_STREAM0);
    // dma_set_peripheral_size(DMA2, DMA_STREAM0, DMA_SxCR_PSIZE_8BIT);
    // dma_set_memory_size(DMA2, DMA_STREAM0, DMA_SxCR_MSIZE_8BIT);
    // dma_set_priority(DMA2, DMA_STREAM0, DMA_SxCR_PL_HIGH);
    // dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM0);
    // dma_channel_select(DMA2, DMA_STREAM0, DMA_SxCR_CHSEL_3);
    // dma_enable_stream(DMA2, DMA_STREAM0);

    // spi_enable_tx_dma(SPI1);
    // spi_enable_rx_dma(SPI1);
}

// конфигурируем выводы для управления датчиком (reset и npd)
void SetupPeriph::ADNS3080pinsSetup() {
    // датчик 1

    // ВЫВОД ДЛЯ РЕСЕТ И НПД МОЖНО ПЕРЕДАТЬ В КОНСТРУКТОРЕ!!!!!!!!!!!!!!

    // настраиваем вывод NPD для нормальной работы датчика
    // этот и следующий выводы управляются программно в ADNS3080.cpp
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
    gpio_set(GPIOC, GPIO0);
    // настраиваем вывод RESET и устанавливаем его в ноль
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_clear(GPIOC, GPIO1);

    // датчик 2
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO2);
    gpio_set(GPIOC, GPIO2);
    // настраиваем вывод RESET и устанавливаем его в ноль
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO3);
    gpio_clear(GPIOC, GPIO3);
}

// конфигурация DMA1 для отправки данных по USART2
void SetupPeriph::DMA1_USART2_Tx_Write(uint8_t *data, uint16_t size) {
    // используем поток 6, канал 4 для USART2_TX
    // dma_stream_reset(DMA1, DMA_STREAM6);
    // dma_set_peripheral_address(DMA1, DMA_STREAM6, (uint32_t)&USART2_DR);
    // dma_set_memory_address(DMA1, DMA_STREAM6, (uint32_t)data);
    // dma_set_number_of_data(DMA1, DMA_STREAM6, size);
    // dma_set_transfer_mode(DMA1, DMA_STREAM6, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    // dma_enable_memory_increment_mode(DMA1, DMA_STREAM6);
    // dma_set_peripheral_size(DMA1, DMA_STREAM6, DMA_SxCR_PSIZE_8BIT);
    // dma_set_memory_size(DMA1, DMA_STREAM6, DMA_SxCR_MSIZE_8BIT);
    // dma_set_priority(DMA1, DMA_STREAM6, DMA_SxCR_PL_MEDIUM);
    // dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM6);
    // dma_channel_select(DMA1, DMA_STREAM6, DMA_SxCR_CHSEL_4);
    // dma_enable_stream(DMA1, DMA_STREAM6);
    // usart_enable_tx_dma(USART2);
}

// запуск режима Motion Burst
// void SetupPeriph::startMotionBurstDMA() {
//     // опускаем линию
//     gpio_clear(GPIOA, GPIO4);
//     // отправляем адрес регистра для его чтения
//     spi_send(SPI1, ADNS3080_MOTION_BURST);
//     // ждем окончания отправки
//     while (!(SPI_SR(SPI1) & SPI_SR_TXE));
// 	// ждем реакции датчика
// 	sensor.delay_us(ADNS3080_T_SRAD_MOT);
// 	// запускаем DMA для приема данных
//     DMA2_SPI1_Rx_Recv(zeroDataOutput, dataBufferInput, 7);
// }

// обрабатываем полученный символ
// void SetupPeriph::Processing_recv_data(uint8_t symbol) {
//     // отправляем команды датчику команды датчику
//     switch (symbol)
//         {
//         // выбираем режим Motion Burst
//         case 's':
//             static uint8_t welcome1[] = "\r\nMotion Burst mode selected\r\n";
//             DMA1_USART2_Tx_Write(welcome1, sizeof(welcome1));
//             // отправляем команду датчику и читаем данные о поверхности
//             startMotionBurstDMA();
//         // запрашиваем у датчика id
//         case 'i':
//             responseID();
//         break;
//     }
// }

void SetupPeriph::SPI1_Setup() {
    // конфигурируем порты SPI1 на альтернативные функции:
    // SPI1_NC (SPI1_NSS) = PA4
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);
    gpio_set(GPIOA, GPIO4);

    // SPI1_SCK = PA5, SPI1_MISO = PA6, SPI1_MOSI = PA7
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO6 | GPIO7);
    gpio_set_af(GPIOA, GPIO_AF5, GPIO5 | GPIO6 | GPIO7);

    // SPI1_MOSI = PB5
    // gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
    // gpio_set_af(GPIOB, GPIO_AF5, GPIO5);

    // перезапускаем SPI1
    rcc_periph_reset_pulse(RST_SPI1);

    // настраиваем SPI1 как мастер
    spi_init_master(SPI1, SPI_CR1_BAUDRATE_FPCLK_DIV_64, 
                    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_2, 
                    SPI_CR1_DFF_8BIT, 
                    SPI_CR1_MSBFIRST);

    // управляем NSS программно
    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1);

    // включаем SPI1
    spi_enable(SPI1);
}

void SetupPeriph::SPI3_Setup() {
    // конфигурируем вывод chip select
    // SPI3_NC (SPI3_NSS) = PA15
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO15);

    // конфигурируем выводы тактирования, miso и mosi
    // SPI3_SCK = PB3, SPI3_MISO = PB4, SPI3_MOSI = PB5
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3 | GPIO4 | GPIO5);
    gpio_set_af(GPIOB, GPIO_AF6, GPIO3 | GPIO4 | GPIO5);

    // перезапускаем SPI3
    rcc_periph_reset_pulse(RST_SPI3);

    // настраиваем SPI3 как мастер
    spi_init_master(SPI3, SPI_CR1_BAUDRATE_FPCLK_DIV_64, 
                    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_2, 
                    SPI_CR1_DFF_8BIT, 
                    SPI_CR1_MSBFIRST);

    // управляем NSS программно
    spi_enable_software_slave_management(SPI3);
    spi_set_nss_high(SPI3);

    // включаем SPI3
    spi_enable(SPI3);
}

void SetupPeriph::USART2_Setup() {
    // настраиваем вывод USART2_TX
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
    // настраиваем вывод USART2_RX
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO3);

    // настраиваем UART
    usart_set_baudrate(USART2, BAUD_SPEED);
    usart_set_databits(USART2, WORD_SIZE);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    // включаем USART2
    usart_enable(USART2);

    // включаем прерывания по принятии данных
    usart_enable_rx_interrupt(USART2);
}

// настраиваем прерывания
void SetupPeriph::Interrupt_Setup() {
    // приоритеты прерываний USART2
    nvic_set_priority(NVIC_USART2_IRQ, 0);
    nvic_enable_irq(NVIC_USART2_IRQ);
    // приоритеты DMA2
    nvic_set_priority(NVIC_DMA2_STREAM0_IRQ, 1);
    nvic_enable_irq(NVIC_DMA2_STREAM0_IRQ);
    // приоритеты DMA1
    nvic_set_priority(NVIC_DMA1_STREAM6_IRQ, 2);
    nvic_enable_irq(NVIC_DMA1_STREAM6_IRQ);
}