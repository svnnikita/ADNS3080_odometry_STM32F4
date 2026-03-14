#include <libopencm3/stm32/rcc.h> 
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <cstdio>

#include "ADNS3080/ADNS3080.hpp"
#include "setup/setup.hpp"

// Схема подключения:
//      Датчик  STM32F407
//       NСS       PB9
//       SCK       PB10
//       MISO      PB14
//       MOSI      PB15
//       RST       PA10

// преобразование пикселя в символ
// строка символов представляет собой значения псевдографики
char pixelSymbol(int k) {
    constexpr char scale[] = "#987654321-,.'` ";  // 16 символов
    return scale[k >> 4];                         // сдвигаем справо на 4 бита
}

int main(void) {
    Clock_Setup();
    Timer_Setup();
    SPI2_Setup();
    USART2_Setup();

    ADNS3080 sensor;
    sensor.setup();
    for (int i = 0; i < 50000; i++) __asm__("nop"); // примерно 50 мс при 168 МГц
    // if(sensor.setup())     // настраиваем датчик
    //     usart_send_blocking( USART2, '1');
    // else
    //     usart_send_blocking( USART2, '0');

    uint8_t product_value = sensor.readRegister(ADNS3080_PRODUCT_ID);
    char buf[32];
    snprintf(buf, sizeof(buf), "ID = 0x%02X\r\n", product_value);
    for (char *p = buf; *p; p++) usart_send_blocking(USART2, *p);

    while (1) {
        // // массив для кадра
        // uint8_t frame[ADNS3080_PIXELS][ADNS3080_PIXELS];
        
        // // принимаем кадр
        // sensor.frameCapture(frame);

        // // ПЕРЕСЫЛАЕМ КАДР ПО UART
        // usart_send_blocking( USART2, '\n' );
        // usart_send_blocking( USART2, '\r' );

        // // проходим по всему массиву
        // for ( uint8_t i = 0; i < ADNS3080_PIXELS; i++ ) {
        //     // Для каждого пикселя в строке
        //     for (uint8_t j = 0; j < ADNS3080_PIXELS; j++) {
        //         usart_send_blocking( USART2, pixelSymbol(frame[i][j] ));
        //     }
        //     usart_send_blocking( USART2, '\n' );
        //     usart_send_blocking( USART2, '\r' );
        // }
        // usart_send_blocking( USART2, '\n' );
        // usart_send_blocking( USART2, '\r' );
        
        // // Активируем Slave
        // gpio_clear(GPIOB, GPIO9);
        
        // // Мигаем светодиодом
        // gpio_toggle(GPIOA, GPIO9);
        
        // uint8_t symbol = 'a'; 
        // spi_send(SPI2, symbol);
        // usart_send(USART2, symbol);
        
        // // Ждем пока данные не будут получены
        // while (!(SPI_SR(SPI2) & SPI_SR_RXNE));
        
        // // Деактивируем Slave
        // gpio_set(GPIOB, GPIO9);
            
        // for ( volatile uint32_t i =0; i < 2'000'000; i++ ) {
        //     __asm__("NOP");
        // }
    }
}