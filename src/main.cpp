#include <libopencm3/stm32/rcc.h> 
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <cstdio>

#include "ADNS3080/ADNS3080.hpp"
#include "setup/setup.hpp"

// объект конфигуратора периферии
SetupPeriph peripheral = SetupPeriph();

// создадим структуру с наименованиями выводов левого датчика
const ADNS3080::Adns3080Pins l_camera_pins = {
    .cs_gpio_port = GPIOA,      
    .cs_gpio_pin = GPIO4,
    .spi = SPI1,
    .reset_gpio_port = GPIOB,
    .reset_gpio_pin = GPIO0
};
// объект левой камеры
ADNS3080 l_camera = ADNS3080(l_camera_pins);

// // создадим структуру с наименованиями выводов правого датчика
// const ADNS3080::Adns3080Pins r_camera_pins = {
//     .cs_gpio_port = GPIOD,      
//     .cs_gpio_pin = GPIO0,
//     .spi = SPI3,
//     .reset_gpio_port = GPIOD,
//     .reset_gpio_pin = GPIO1
// };
// // объект правой камеры
// ADNS3080 r_camera = ADNS3080(r_camera_pins);


int main(void) {
    // сконфигурируем датчик
    // включаем подсветку и устанавливаем высокое разрешение
    l_camera.setup(true, true);

    l_camera.delay_us(ADNS3080_T_SWW);
    
    // volatile uint8_t result1 = l_camera.readRegister(ADNS3080::ADNS3080_CONFIGURATION_BITS);

    // l_camera.delay_us(ADNS3080_T_SWW);

    volatile uint8_t result = l_camera.readRegister(ADNS3080::ADNS3080_PRODUCT_ID);

    l_camera.delay_us(ADNS3080_T_SWW);

    if (result == ADNS3080::ADNS3080_PRODUCT_ID_VALUE) {
        usart_send_blocking(USART2, 't');
    } else { 
        usart_send_blocking(USART2, 'f');
    }
        
    while (1) {
        // usart_send_blocking(USART2, 'A');
        // for (volatile uint32_t i = 0; i < 1000000; i++);
    }
}