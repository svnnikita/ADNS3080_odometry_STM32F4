// #include "ADNS3080/ADNS3080.hpp"

// константы для инициализации UART
constexpr uint32_t BAUD_SPEED{115200};  // скорость передачи данных
constexpr uint8_t WORD_SIZE{8};

// создадим класс для инициализации необходимой периферии мк
class SetupPeriph{
    // первый конструктор
    // передаем в конструктор константы, необходимые для работы SPI1 и SPI3 
    SetupPeriph(uint32_t SPIX1);
public:
    void Clock_Setup();
    void Timer_Setup();
    void DMA2_SPI1_Rx_Recv(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t size); // получаем данные по SPI1      
    void DMA1_USART2_Tx_Write(uint8_t *data, uint16_t size);    // отправляем данные по USART2
    // void startMotionBurstDMA();
    // void Processing_recv_data(uint8_t symbol);
    void ADNS3080pinsSetup();

    // конфигурируем SPI для двух датчиков
    void SPI1_Setup();
    void SPI3_Setup();
    
    void USART2_Setup();
    void Interrupt_Setup();
    // void responseID();
};