/* Хэдер-файл библиотеки ADNS3080.h */
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

#ifndef ADNS3080_h
#define ADNS3080_h 

//------------ Constants and registers ---------------
//-------------- Константы и регистры ---------------

// Signal delay time:
// Время задержки сигналов:
constexpr uint16_t ADNS3080_T_IN_RST          {500};  // время задержки после сброса (мкс)
constexpr uint8_t ADNS3080_T_PW_RESET         {10};   // длительность импульса сброса (мкс)

// Задержка между отправкой адреса регистра и началом чтения данных (мкс)
// Константа применяется для регистров движения (Motion registers)
constexpr uint8_t ADNS3080_T_SRAD_MOT         {75};   

// Интервал времени между командами записи (мкс)
constexpr uint8_t ADNS3080_T_SWW              {50};

// Задержка между отправкой адреса регистра и началом чтения данных (мкс)
// Константа применяется для всех регистров, кроме регистров движения (см. выше)
constexpr uint8_t ADNS3080_T_SRAD             {50};

// Задержка побайтного захвата кадров при загрузке и повторной загрузке SROM (мкс)
constexpr uint8_t ADNS3080_T_LOAD             {10};

// Время, в течение которого необходимо выдержать высокий уровень сигнала 
// при выходе из режима frame capture
constexpr uint8_t ADNS3080_T_BEXIT            {4};

// Pixel dimensions:
constexpr uint8_t ADNS3080_PIXELS             {30};   // разрешение датчика

// Количество всех данных в режиме Motion Burst
constexpr uint8_t MOT_BURST_ALL_DATA	      {7};

// Количество данных о перемещении в режиме Motion Burst
constexpr uint8_t MOT_BURST_DISPL_DATA	      {2};

// Registers:
// Доступ к регистрам ADNS-3080 осуществляется через последовательный порт. 
// Регистры используются для считывания данных о движении и состоянии,
// а также для настройки конфигурации устройства.
constexpr uint8_t ADNS3080_PRODUCT_ID        {0x00}; 	// используется для проверки соединения
														// см. ADNS3080_PRODUCT_ID_VALUE

constexpr uint8_t ADNS3080_CONFIGURATION_BITS{0x0a};  	// используется для конфигурации датчика

// Запись любого значения в этот регистр приведет к очистке регистров Delta_X, 
// Delta_Y и внутреннего движения. Используется как быстрый способ обнуления
// счетчиков движения без перезагрузки всего чипа.
constexpr uint8_t ADNS3080_MOTION_CLEAR      {0x12};

// Запись 0x83 в этот регистр приведет к тому, что следующие доступные полные одна целая
// две трети кадра значений в пикселях будут сохранены в SROM. Запись в этот регистр 
// требуется перед использованием режима Frame Capture для считывания значений пикселей
constexpr uint8_t ADNS3080_FRAME_CAPTURE     {0x13};

// Регистр используется для высокоскоростного доступа ко всем значениям пикселей из 
// одного и 2/3 полного кадра.
constexpr uint8_t ADNS3080_PIXEL_BURST       {0x40};

// Регистр используется для высокоскоростного доступа к регистрам Motion, Delta_X и 
// Delta_Y, SQUAL, Shutter_ Upper, Shutter_Lower и Maximum_Pixel.
constexpr uint8_t ADNS3080_MOTION_BURST      {0x50};

// Содержит уникальный идентификатор, присвоенный ADNS-3080
constexpr uint8_t ADNS3080_PRODUCT_ID_VALUE  {0x17};

class ADNS3080 {
	// передадим в конструктор параметры для конфигурации периферии
	ADNS3080(uint32_t cs_gpio_port,		// группа выводов, на которой расположен используемый SPI
			 uint16_t cs_gpio_pin,		// вывод, на котором расположен Chip Select
		     uint32_t spi,				// номер используемого SPI
			 uint32_t reset_gpio_port,	// группа выводов, на котором расположен вывод reset
			 uint16_t reset_gpio_pin);	// вывод, на котором расположен reset
private:
	// сведения об используемом SPI получаем из конструктора
	uint32_t cs_gpio_port;
	uint16_t cs_gpio_pin;
	uint32_t spi;
	// данные об выводе для перезагрузки датчика
	uint32_t reset_gpio_port;
	uint16_t reset_gpio_pin;
	
	// опускаем chip select для связи с датчиком
	void csLow() { gpio_clear( cs_gpio_port, cs_gpio_pin ); }
	// поднимаем chip select для завершения связи
	void csHigh() { gpio_set( cs_gpio_port, cs_gpio_pin); }
	// перезагружаем датчик
	void reset() 
	{
		// подаем на вывод перезагрузки высокий сигнал
		gpio_set(reset_gpio_port, reset_gpio_pin);
		// ждем реакции датчика
		delay_us(ADNS3080_T_PW_RESET);
		// опускаем сигнал
		gpio_clear(reset_gpio_port, reset_gpio_pin);
		// ждем реакции датчика
		delay_us(ADNS3080_T_SRAD);      
	}

	// чтение регистров датчика
	uint8_t readRegister( const uint8_t );
	// запись в регистры датчика
	void writeRegister( const uint8_t, uint8_t );
	
public:
	// инициализация датчика
	bool setup(const bool=false, const bool=false);
	// функция задержки для корректной работы датчика
	void delay_us( uint16_t delay );

	// очистка регистра данных о перемещении
	void motionClear();
	// запуск передачи данных о качестве поверхности и перемещении
	void motionBurst(uint8_t *buffer);
	// запуск передачи данных только о перемещении
	// void displacement();
	// запуск передачи сырого изображения с датчика
	void frameCapture( uint8_t[ADNS3080_PIXELS][ADNS3080_PIXELS] );
};

#endif