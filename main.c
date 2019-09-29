#include <util/delay.h>
#include <avr/io.h>

#include "settings.h"
#include "soft_uart.h"
#include "tiny_modbus_rtu_slave.h"

unsigned int read_reg(unsigned int address){
    if ((address>=M90E26_START_ADDRESS) && (address<=M90E26_END_ADDRESS)) {
        return 1;
    } 
    else if (address == RELAY_REG_ADDRESS) {
        return 2;
    }
    else if ((address>=DS18B20_START_ADDRESS) && (address<=DS18B20_END_ADDRESS)) {
        return 3;
    }
    else if ((address>=ERRORS_START_ADDRESS) && (address<=ERRORS_END_ADDRESS)) {
        return 4;
    }
    else 
        return 0;
}

bool write_reg(unsigned int address, unsigned int data){
    if ((address>=M90E26_START_ADDRESS) && (address<=M90E26_END_ADDRESS)) {
        return true;
    } 
    else if (address == RELAY_REG_ADDRESS) {
        return true;
    }
    else if ((address>=DS18B20_START_ADDRESS) && (address<=DS18B20_END_ADDRESS)) {
        return true;
    }
    else if ((address>=ERRORS_START_ADDRESS) && (address<=ERRORS_END_ADDRESS)) {
        return true;
    }
    else 
        return false;
}

void led_set(bool stat) {
    if (stat) {
        PORTB |= (1 << PB2);
    }
    else {
        PORTB &= ~(1 << PB2);
    }
}

int
main(void)
{
    DDRB = (1 << PB2);

    slaveID = 0x10;
    modbus_uart_putc = &uart_putc;
    modbus_read_reg = &read_reg;
    modbus_write_reg = &write_reg;
    modbus_led = &led_set;

    /* loop */
    while (1) {
        pull_port(uart_getc());
    }
}

