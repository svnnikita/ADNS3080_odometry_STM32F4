#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
/* Хэдэр для конфигурации микроконтроллера */

constexpr uint32_t BAUD_SPEED{115200};  // скорость передачи данных
constexpr uint8_t WORD_SIZE{8};

// создадим класс для инициализации необходимой периферии мк
class SetupPeriph 
{	 
public:
    // конструктор с конфигурацией
    SetupPeriph() 
    {
        Clock_Setup();
        Timer_Setup();
        SPI1_Setup();
        // SPI2_Setup();
        SPI3_Setup();
        USART2_Setup();
        ADNS3080PinsSetup();
        
        // отправим сообщение для отладки
        // usart_send_blocking(USART2, 'S');
    }


    // тактирование периферии
    void Clock_Setup();

    // конфигурация таймера TIM6
    void Timer_Setup();

    // получаем данные по SPI1
    // void DMA2_SPI1_Rx_Recv(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t size);
    
    // отправляем данные по USART2
    // void DMA1_USART2_Tx_Write(uint8_t *data, uint16_t size);
    
    // получаем данные при помощи DMA
    // void startMotionBurstDMA();

    // обработка данные от DMA
    // void Processing_recv_data(uint8_t symbol);

    // конфигурируем вспомогательные выводы датчика
    void ADNS3080PinsSetup();

    // конфигурируем SPI для двух датчиков
    void SPI1_Setup();
    void SPI2_Setup();
    void SPI3_Setup();
    
    void USART2_Setup();
    // void Interrupt_Setup();
    // void responseID();
};