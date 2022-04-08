/*
	Program: Keypad test code using ATmega32a
	Pins to Use:
		For the keypad:
			Check the keypad.h header file
		For the output:
			Check the seven_seg.h header file
*/

#include <avr/io.h>
#include "keypad.h"

void display_on_lcd(){
	if(updated){
		lcd_clear();
		if(typing_number){
			lcd_print("target: ");
			char tgt[3] = {'0' + num_to_send,0,0};
			lcd_print(tgt);
		}
		else{
			lcd_print(pressed_buttons);
		}
		updated = 0;
	}
}

int main () {
	//Initializing main system timer
	init_main_timer();
	//Initializing keypad
	init_keypad();

	//Setting up the output
	lcd_init();
	lcd_print("enter message");
	//Main loop
	while (1){
		check_keypad();
		display_on_lcd();
	}
	//End main loop
}
