/*
	Program: Keypad test code using ATmega32a
	Pins to Use:
		For the keypad:
			Check the keypad.h header file
		For the output:
			Check the seven_seg.h header file
*/

#include <avr/io.h>
#include "keypad_alvyne.h"
#include"../lcd/lcd_i2c.h"
#include"../lcd/main_timer.h"


char to_display[10];


void validate_password(){
	to_display[0] = typed_password[0];
	to_display[1] = typed_password[1];
	to_display[2] = typed_password[2];
	to_display[3] = typed_password[3];
}

void detected_new_press(){
	to_display[0] = typed_password[0];
	to_display[1] = typed_password[1];
	to_display[2] = typed_password[2];
	to_display[3] = typed_password[3];
}



int main () {
	//Initializing main system timer
	init_main_timer();
	//Initializing keypad
	init_keypad();

	//Setting up the output
	lcd_init();

	//Main loop
	while (1){
		check_keypad();
		lcd_print(typed_password);
	}
	//End main loop
}
