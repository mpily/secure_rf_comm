avr-gcc -Wall -g -Os -mmcu=atmega32 -o test1.bin lcd_i2c_test.c
avr-objcopy -j .text -j .data -O ihex test1.bin test1.hex
avrdude -p atmega32 -c avrisp -U flash:w:test1.hex:i -F -P /dev/ttyACM0 -b 19200
