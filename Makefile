MCU   = atmega328p
F_CPU = 16000000UL
PORT  = COM4


CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=gnu11 -Iinc
SRC = $(wildcard src/*.c)

all: main.hex


main.elf: $(SRC)
	avr-gcc $(CFLAGS) $^ -o $@

main.hex: main.elf
	avr-objcopy -O ihex -R .eeprom $< $@
	avr-size --format=berkeley $<


flash: main.hex
	avrdude -C avrdude.conf -c arduino -p m328p -P $(PORT) -b 115200 -U flash:w:main.hex:i

clean:
	rm -f *.elf *.hex