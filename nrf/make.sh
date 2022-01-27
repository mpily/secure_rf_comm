#avr-gcc -c -mmcu=atmega32  NRFLite.c -o nrf.o
avr-gcc -c -mmcu=atmega32  test.c -o test1.o
avr-gcc -mmcu=atmega32 test1.o   -o test1.elf
avr-objcopy -O ihex -j .text -j .data test1.elf test1.hex
avrdude -p atmega32 -c avrisp -U flash:w:test1.hex:i -F -P /dev/ttyACM0 -b 19200
