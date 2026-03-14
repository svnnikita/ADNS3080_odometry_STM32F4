/* Хэдер-файл библиотеки ADNS3080.h */
#include <libopencm3/stm32/spi.h>

#ifndef ADNS3080_h
#define ADNS3080_h 

//------------ Constants and registers ---------------
//-------------- Константы и регистры ---------------

// Signal delay time:
// Время задержки сигналов:
#define ADNS3080_T_IN_RST             500  // время задержки после сброса (мкс)
#define ADNS3080_T_PW_RESET           10   // длительность импульса сброса (мкс)

// Задержка между отправкой адреса регистра и началом чтения данных (мкс)
// Константа применяется для регистров движения (Motion registers)
#define ADNS3080_T_SRAD_MOT           75   

// Интервал времени между командами записи (мкс)
#define ADNS3080_T_SWW                50

// Задержка между отправкой адреса регистра и началом чтения данных (мкс)
// Константа применяется для всех регистров, кроме регистров движения (см. выше)
#define ADNS3080_T_SRAD               50

// Задержка побайтного захвата кадров при загрузке и повторной загрузке SROM (мкс)
#define ADNS3080_T_LOAD               10

// Время, в течение которого необходимо выдержать высокий уровень сигнала 
// при выходе из режима frame capture
#define ADNS3080_T_BEXIT              4

// Pixel dimensions:
#define ADNS3080_PIXELS               30   // разрешение датчика

// Registers:
// Доступ к регистрам ADNS-3080 осуществляется через последовательный порт. 
// Регистры используются для считывания данных о движении и состоянии,
// а также для настройки конфигурации устройства.
#define ADNS3080_PRODUCT_ID           0x00  // используется для проверки соединения
											// см. ADNS3080_PRODUCT_ID_VALUE

#define ADNS3080_CONFIGURATION_BITS   0x0a  // используется для конфигурации датчика

// Запись любого значения в этот регистр приведет к очистке регистров Delta_X, 
// Delta_Y и внутреннего движения. Используется как быстрый способ обнуления
// счетчиков движения без перезагрузки всего чипа.
#define ADNS3080_MOTION_CLEAR         0x12

// Запись 0x83 в этот регистр приведет к тому, что следующие доступные полные одна целая
// две трети кадра значений в пикселях будут сохранены в SROM. Запись в этот регистр 
// требуется перед использованием режима Frame Capture для считывания значений пикселей
#define ADNS3080_FRAME_CAPTURE        0x13

// Регистр используется для высокоскоростного доступа ко всем значениям пикселей из 
// одного и 2/3 полного кадра.
#define ADNS3080_PIXEL_BURST          0x40

// Регистр используется для высокоскоростного доступа к регистрам Motion, Delta_X и 
// Delta_Y, SQUAL, Shutter_ Upper, Shutter_Lower и Maximum_Pixel.
#define ADNS3080_MOTION_BURST         0x50

// Содержит уникальный идентификатор, присвоенный ADNS-3080
#define ADNS3080_PRODUCT_ID_VALUE     0x17


//--------------- Template Parameters ---------------- [       No characters after backlash!       ]
//--------------- Шаблонные параметры ---------------- [ Символы после обратного слэша не ставить! ]
// Определяем список параметров
// #define TEMPLATE_TYPE           \
//         uint8_t PIN_RESET,      \
//         uint8_t PIN_NCS         
    
// #define TEMPLATE_INPUTS         \
//                 PIN_RESET,      \
//                 PIN_NCS 


// === РЕШИТЬ НУЖНЫ ЛИ ШАБЛОННЫЕ ПАРАМЕТРЫ ИЛИ НЕТ (4.11.25) ===
//template <TEMPLATE_TYPE>
class ADNS3080 {  
    private:
		// Read and write registers:
		// Запись и чтение регистров. Методы используются внутри открытых методов.
		void writeRegister( const uint8_t, uint8_t );
		
		
	public: 
		uint8_t readRegister( const uint8_t );
	
		void reset();
		bool setup( const bool=false, const bool=false );
		void delay_us(uint16_t delay);
		//void motionClear();
		
		// Major outputs:
		//void motionBurst( uint8_t*, int8_t*, int8_t*, uint8_t*, uint16_t*, uint8_t* );
		//void displacement( int8_t*, int8_t* );
		void frameCapture( uint8_t[ADNS3080_PIXELS][ADNS3080_PIXELS] );
};

#endif