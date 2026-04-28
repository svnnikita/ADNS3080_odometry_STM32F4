// ethernet_config.hpp -- заголовочный файл для 
// конфигурации передачи данных по Ethernet

#include <libopencm3/stm32/usart.h>
#include <cstdio>

#define UID_BASE 0x1FFF7A10

class EthernetConfig {
public:
    // конструктор
    EthernetConfig() {

    }

    // узнаем уникальный серийный номер мк
    void Read_UID(void)
    {
        uint16_t *idBase0 = (uint16_t*)UID_BASE;
        uint16_t *idBase1 = (uint16_t*)(UID_BASE + 0x02);
        uint32_t *idBase2 = (uint32_t*)(UID_BASE + 0x04);
        uint32_t *idBase3 = (uint32_t*)(UID_BASE + 0x08);

        char buffer[64];
        uint32_t len = sprintf(buffer, "UID %04x-%04x-%08lx-%08lx\n", 
                        *idBase0, *idBase1, (unsigned long)*idBase2, (unsigned long)*idBase3);
        // Отправка через USART1 (предварительно должен быть инициализирован)
        for (uint32_t i = 0; i < len; i++) {
            usart_send_blocking(USART2, buffer[i]);
        }
    }

    // устанавливаем уникальный MAC-адрес
    void Set_MAC_Address() {

    }

private:
    //  MAC-адрес, задается произвольно
    // СОЗДАЕМ МАК АДРЕС НА ОСНОВЕ АЙДИ МК
    // ПРИМЕНЯЕМ ФУНКЦИЮ eth_set_mac()

    
};