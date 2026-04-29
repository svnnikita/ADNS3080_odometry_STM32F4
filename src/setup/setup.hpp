#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
/* Хэдэр для конфигурации микроконтроллера */

constexpr uint32_t BAUD_SPEED = 115200;  // скорость передачи данных
constexpr uint8_t WORD_SIZE = 8;

constexpr uint16_t T_nRSTIA = 400;  // время удержания nRST в активном состоянии (0), мкс
constexpr uint16_t T_AFTERnRST = 5000; // время удержания nRST в активном состоянии (0), мкс

// создадим класс для инициализации необходимой периферии мк
class SetupPeriph 
{	 
public:
    // конструктор с конфигурацией
    SetupPeriph() 
    {
        clockSetup();
        timerSetup();
        macSetup();
        spi1Setup();
        spi3Setup();
        usart2Setup();
        adns3080PinsSetup();
    }

    // тактирование периферии
    void clockSetup();

    // конфигурация таймера TIM6
    void timerSetup();

    // временная задержка (мкс)
    void delayUs(uint16_t delay);

    // конфигурация портов MAC контроллера
    void macSetup();

    // получаем данные по SPI1
    // void DMA2_SPI1_Rx_Recv(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t size);
    
    // отправляем данные по USART2
    // void DMA1_USART2_Tx_Write(uint8_t *data, uint16_t size);
    
    // получаем данные при помощи DMA
    // void startMotionBurstDMA();

    // обработка данные от DMA
    // void Processing_recv_data(uint8_t symbol);

    // конфигурируем вспомогательные выводы датчика
    void adns3080PinsSetup();

    // конфигурируем SPI для двух датчиков
    void spi1Setup();
    void spi3Setup();
    
    void usart2Setup();
    // void Interrupt_Setup();
};