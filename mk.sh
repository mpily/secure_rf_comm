avr-gcc -c -mmcu=atmega32  endequip.c -o test1.o
avr-gcc -mmcu=atmega32 test1.o   -o test1.elf
avr-objcopy -O ihex -j .text -j .data test1.elf test1.hex
avrdude -p atmega32 -c avrisp -U flash:w:test1.hex:i  -P /dev/ttyACM0 -b 19200
#avr-size -C --mcu=atmega168 project.elf to check size of code
