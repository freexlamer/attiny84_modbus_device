
#include "one_wire.h"

// Инициализация DS18B20
unsigned char DS18B20_init(void)
{
	unsigned char OK_Flag;

	DS18B20_DDR |= (1 << DS18B20_DQ); // PC0 - выход
	DS18B20_PORT &= ~(1 << DS18B20_DQ); // Устанавливаем низкий уровень
	_delay_us(490);
	DS18B20_DDR &= ~(1 << DS18B20_DQ); // PC0 - вход
	_delay_us(68);
	OK_Flag = (DS18B20_PIN & (1 << DS18B20_DQ)); // Ловим импульс присутствия датчика
	// если OK_Flag = 0 датчик подключен, OK_Flag = 1 датчик не подключен
	_delay_us(422);
	return OK_Flag;
}

// Функция чтения байта из DS18B20
unsigned char DS18B20_read(void)
{
	unsigned char i, data = 0;
	for(i = 0; i < 8; i++)
	{
		DS18B20_DDR |= (1 << DS18B20_DQ); // PC0 - выход
		_delay_us(2);
		DS18B20_DDR &= ~(1 << DS18B20_DQ); // PC0 - вход
		_delay_us(4);
		data = data >> 1; // Следующий бит
		if(DS18B20_PIN & (1 << DS18B20_DQ)) data |= 0x80;
		_delay_us(62);
	}
	return data;
}

// Функция записи байта в DS18B20
void DS18B20_write(unsigned char data)
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		DS18B20_DDR |= (1 << DS18B20_DQ); // PC0 - выход
		_delay_us(2);
		if(data & 0x01) DS18B20_DDR &= ~(1 << DS18B20_DQ);
		else DS18B20_DDR |= (1 << DS18B20_DQ);
		data = data >> 1; // Следующий бит
		_delay_us(62);
		DS18B20_DDR &= ~(1 << DS18B20_DQ); // PC0 - вход
		_delay_us(2);
	}
}                       // разрешаем прерывания

bool DS18B20_get(unsigned int *data, unsigned char *cfg) {

	unsigned char data_h, data_l;

	DS18B20_init();
	DS18B20_write(0xCC); // Проверка кода датчика
	DS18B20_write(0x44);     // Запуск температурного преобразования
	_delay_ms(250);

	DS18B20_init();
	DS18B20_write(0xCC); // Проверка кода датчика
	DS18B20_write(0xBE);     // Считываем содержимое ОЗУ

	data_l = DS18B20_read(); // Читаем первые 2 байта блокнота
	data_h = DS18B20_read();
	DS18B20_read();
	DS18B20_read();
	*cfg = DS18B20_read();

	_delay_ms(250);

	*data = data_h << 8 | data_l;

	return true;

}
