
## Toolchain

avr-gcc 4.9.2


### To build

`make clean && make`

## Programmer

avrdude 6.2

Was used USBtinyISP.
To use another change `PROGRAMMER` in `Makefile`.

### To flash firmware

`make flash`

### To flash fuses for internal RC oscillator

`make fuse`

### To flash fuses for external crystal 8 MHz

`make fuse_extosc`

## MCU

### resources

TC0 - internal clock `millis()`
TC1 - internal RC oscillator calibration


