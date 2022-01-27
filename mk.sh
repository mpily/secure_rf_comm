avr-gcc -Wall -g -Os -mmcu=atmega32 -o test1.bin test1.c
avr-objcopy -j .text -j .data -O ihex test1.bin test1.hex
avrdude -p atmega32 -c avrisp2 -U flash:w:test1.hex:i -F -P usb
