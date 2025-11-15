/* Хэдэр для конфигурации микроконтроллера */

constexpr uint32_t BAUD_SPEED{115200};  // скорость передачи данных
constexpr uint8_t WORD_SIZE{8};

void Clock_Setup(); // тактирование
void Timer_Setup(); // конфигурация таймера
void SPI2_Setup();
void USART2_Setup();