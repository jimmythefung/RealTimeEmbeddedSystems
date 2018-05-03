PORT ?= /dev/ttyACM0
MCU=atmega32u4
CFLAGS=-g -Wall -mcall-prologues -mmcu=$(MCU) -Os
LDFLAGS=-Wl,-gc-sections -Wl,-relax

ifndef VIRTUAL_SERIAL_PATH
$(warning VIRTUAL_SERIAL_PATH must be set to use usb virtual serial)
else
CFLAGS+= -I$(VIRTUAL_SERIAL_PATH) -DVIRTUAL_SERIAL
LDFLAGS+=  -L$(VIRTUAL_SERIAL_PATH) -lVirtualSerial
endif


CC=avr-gcc
TARGET=main
OBJECT_FILES=main.o

all: $(TARGET).hex

clean:
	rm -f *.o *.hex *.obj *.hex

%.hex: %.obj
	avr-objcopy -R .eeprom -O ihex $< $@

%.obj: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(OBJECT_FILES) $(LDFLAGS) -o $@

program: $(TARGET).hex
	avrdude -p $(MCU) -c avr109 -P $(PORT) -U flash:w:$(TARGET).hex
