#include <libopencm3/stm32/rcc.h> 
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>

#include <cstdio>

#include "ADNS3080/ADNS3080.hpp"
#include "SetupPeriph/SetupPeriph.hpp"

// схема подключения
// SPI1_NC => PA4 (good)
// SPI1_SCK => PA5 (good)
// SPI1_MISO => PA6 (good)
// SPI1_MOSI => PA7 (good)

// настройка периферии
// SetupPeriph setupPer;
// ADNS3080 sensor;

// обработчик прерываний USART2_RX
// void usart2_isr() {
//     if (usart_get_flag(USART2, USART_FLAG_RXNE)) {
//         // принимаем символ
//         uint8_t rec_byte = usart_recv(USART2);
//         // пересылаем его обратно (эхо)
//         setupPer.DMA1_USART2_Tx_Write(&rec_byte, sizeof(rec_byte));
//     }
// }

// буфферы для получения данных
// буффер, откуда будут отправляться нули для инициализации получения данных по SPI1
uint8_t zeroData[MOT_BURST_ALL_DATA] = {0, 0, 0, 0, 0, 0, 0};
// буффер, куда будут записываться данные c датчика
uint8_t rxBuffer[MOT_BURST_ALL_DATA] = {0, 0, 0, 0, 0, 0, 0};

uint8_t txBuffer[MOT_BURST_ALL_DATA] = {0, 0, 0, 0, 0, 0, 0};


// буфферы для id датчика
uint8_t dummy[1] = {0x00};
uint8_t rx_buffer[1] = {0};

// обработчик прерываний DMA2 (SPI1_RX)
// void dma2_stream0_isr() {
//     if (dma_get_interrupt_flag(DMA2, DMA_STREAM0, DMA_TCIF)) {
//         // очищаем бит
//         dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
//         // отключаем потоки
//         dma_disable_stream(DMA2, DMA_STREAM0);
//         dma_disable_stream(DMA2, DMA_STREAM3);
//         // отключаем DMA в SPI1
//         spi_disable_rx_dma(SPI1);
//         spi_disable_tx_dma(SPI1);

//         // поднимаем линию для завершения передачи
//         gpio_set(GPIOA, GPIO4);


//     }
// }

// обработчик прерываний DMA1 (USART2_TX)
// void dma1_stream6_isr() {
//     if (dma_get_interrupt_flag(DMA1, DMA_STREAM6, DMA_TCIF)) {
//         // очищаем бит
//         dma_clear_interrupt_flags(DMA1, DMA_STREAM6, DMA_TCIF);
//         dma_disable_stream(DMA1, DMA_STREAM6);
//         usart_disable_tx_dma(USART2);
//         usartBusy = false;
//     }
// }

int main(void) {
    // настраиваем периферию
    // setupPer.Clock_Setup();
    // setupPer.Timer_Setup();
    // setupPer.SPI1_Setup();
    // setupPer.USART2_Setup();
    // setupPer.Interrupt_Setup();

    // настраиваем датчик
    // static uint8_t welcome[] = "\r\nconfigure the sensor -- done\r\n";
    // static uint8_t err[] = "\r\nconfigure the sensor -- ERROR\r\n";
    // sensor.setup();
    // if (setupPer.sensor.setup())
    //     setupPer.DMA1_USART2_Tx_Write(welcome, sizeof(welcome));
    // else setupPer.DMA1_USART2_Tx_Write(err, sizeof(err));



    while (1) {
        gpio_clear(GPIOA, GPIO4);
        // отправляем адрес регистра (младший бит в нуле -- режим чтения)
        spi_send(SPI1, 0x00);
        while (!(SPI_SR(SPI1) & SPI_SR_RXNE));
        volatile uint8_t output = spi_read(SPI1);
        // ждем, пока освободится шина
        while (SPI_SR(SPI1) & SPI_SR_BSY);
        // отключаем линию
        gpio_set(GPIOA, GPIO4);
        usart_send_blocking(USART2, output);
    }
}