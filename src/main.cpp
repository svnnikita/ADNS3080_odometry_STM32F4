#include <libopencm3/stm32/rcc.h> 
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <cstdio>

#include "ADNS3080/ADNS3080.hpp"
#include "setup/setup.hpp"

// объект конфигуратора периферии
SetupPeriph peripheral = SetupPeriph();

// создадим структуру с наименованиями выводов датчика
const Adns3080Pins l_camera_pins = {
    .cs_gpio_port = GPIOA,      
    .cs_gpio_pin = GPIO4,
    .spi = SPI1,
    .reset_gpio_port = GPIOB,
    .reset_gpio_pin = GPIO0
};

// объект левой камеры
ADNS3080 l_camera = ADNS3080(l_camera_pins);

int main(void) {
    // сконфигурируем датчик
    // включаем подсветку и устанавливаем высокое разрешение
    volatile bool res = l_camera.setup(true, true);
    l_camera.delay_us(ADNS3080_T_SWW);

    // прочитаем id датчика
    const bool led_mode = true; 
    const bool resolution = true;
    uint8_t mask = 0b00000000 | led_mode << 6 | resolution << 4;

    uint8_t result = l_camera.readRegister(ADNS3080_CONFIGURATION_BITS);
    l_camera.delay_us(ADNS3080_T_SWW);

    if (result == mask) {
       usart_send_blocking(USART2, 't');
    } else { 
        usart_send_blocking(USART2, 'f');
    }
        
    while (1) {
        
    }
}