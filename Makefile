# Name: Makefile

# Based on https://github.com/electronut/tinyDriverP
# electronut.in

DEVICE      = attiny84a
CLOCK      = 8000000
PROGRAMMER = -c usbtiny
OBJECTS    = main.o tiny_modbus_rtu_slave.o m90e26.o one_wire.o osc_calibration.o SoftwareSerial.o millis.o
OBJECTS_PREPARE = osc_calibration.o prepare.o

# for ATTiny85 - unset CKDIV8
#int 8MHz
FUSES       = -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m
#ext 8MHz
FUSES_EXTOSC = -U lfuse:w:0xEE:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m
# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE) 
COMPILE = avr-gcc -std=gnu99 -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

flash_prepare:	prepare.hex
	$(AVRDUDE) -U flash:w:prepare.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

fuse_extosc:
	$(AVRDUDE) $(FUSES_EXTOSC)

read_eeprom:
	$(AVRDUDE) -U eeprom:r:eeprom.hex:i

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)
	rm -f prepare.elf prepare.hex

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

prepare.elf: $(OBJECTS_PREPARE)
	$(COMPILE) -o prepare.elf $(OBJECTS_PREPARE)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

prepare.hex: prepare.elf
	rm -f prepare.hex
	avr-objcopy -j .text -j .data -O ihex prepare.elf prepare.hex
	avr-size --format=avr --mcu=$(DEVICE) prepare.elf


# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
