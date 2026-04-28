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

// структура данных о перемещении
ADNS3080::MotionData l_data;

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
    // небольшая задержка для включения датчика
    for (volatile uint32_t i = 0; i < 2000000; i++);
    
    // сконфигурируем датчик
    // включаем подсветку и устанавливаем высокое разрешение
    volatile uint8_t setup = l_camera.setup(true, true);
    l_camera.delay_us(ADNS3080_T_SWW);

    // проверяем корректность подключения
    if (setup == true) {
        usart_send_blocking(USART2, 't');
    } else { 
        usart_send_blocking(USART2, 'f');
    }

    // создаем буффер для создания строки
    char buffer[64];
        
    while (1) {
        // запускаем режим считывания смещения
        l_camera.motionBurst(l_data);

        // формируем строку
        uint32_t len = 
            sprintf(buffer, 
                    "M: %d, X: %4d, Y: %4d, SQ: %3u, SH: %d, MP: %u\r\n", 
                    l_data.motion, l_data.dx, l_data.dy, 
                    l_data.squal, l_data.shutter, l_data.max_pix);
        
        // отправляем данные
        for (uint32_t i = 0; i < len; i++) {
            usart_send_blocking(USART2, buffer[i]);
        }

    }
}