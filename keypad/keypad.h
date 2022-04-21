/*
	based off of keypad library by AlvyneZ: https://github.com/AlvyneZ/Access-Control/blob/main/Keypad%20test/keypad.h
	Header: Library for getting 4-digit passcode input from the 4x4 keypad using an ATmega32a
	Pins to be used:
		Column pins:	Column 1..4	-> pin D0..3 (pins 14..17)
		Row pins:		Row 1..4	-> pin D4..7 (pins 18..21)
*/



#ifndef KEYPAD
//To prevent errors if the header file is included multiple times
#define KEYPAD


#include <avr/io.h>		//Required for Data Direction Registers (DDR)

//The keypad depends on the main timer for its timing functions
#include"../lcd/lcd_i2c.h"
#include"../lcd/main_timer.h"


//The physical keys in order on the 4x4 keypad
const char KEYPAD_KEYS[4][4] = {
		{'1','2','3','A'},
		{'4','5','6','B'},
		{'7','8','9','C'},
		{'*','0','#','D'}
	};
/*const char KEYPAD_KEYS[4][4] = {
		{'7','8','9','A'},
		{'4','5','6','B'},
		{'1','2','3','C'},
		{'*','0','#','D'}
	};*/
	
//The time taken for the system to forget what was typed if no key is pressed
const uint64_t clear_pass_interval = 156250UL;	//5 seconds (@ 8MHz & 256 prescaler)

const char character_order[12][10] = {{'1','1','1','1','1','1','1','1','1','1'},
                                      {'2','a','b','c','A','B','C','2','2','2'},
                                      {'3','d','e','f','D','E','F','3','3','3'},
                                      {'4','g','h','i','G','H','I','4','4','4'},
                                      {'5','j','k','l','J','K','L','5','5','5'},
                                      {'6','m','n','o','M','N','O','6','6','6'},
                                      {'7','p','q','r','s','P','Q','R','S','7'},
                                      {'8','t','u','v','T','U','V','8','8','8'},
                                      {'9','w','x','y','z','W','X','Y','Z','9'},
                                      {'*','*','*','*','*','*','*','*','*','*'},
                                      {'0',' ','.',',','0','0','0','0','0','0'},
                                      {'#','#','#','#','#','#','#','#','#','#'}};
//order of characters pressed by the keypad

                                     
//To hold the time of the last key press
volatile uint64_t previous_key_reg_t = 0;

uint8_t updated = 0;


//To hold the location in typed_password to input the next keyed in character
volatile uint8_t pass_offset = 0;

//To hold the pressed buttons in a certain period for detecting simultaneous presses
char pressed_buttons[40];
char typed_message[40];//when message has finally been written use this to store it

uint8_t num_to_send;//number to send the message_to
int msg_length;//length of the message
uint8_t typing_number;//boolean to tell whether we are typing a number or a message
//To hold the number of detected presses
volatile uint8_t pressed_count = 0;


void get_pos(char x, uint8_t * row, uint8_t* col){
    //given a character ,x, it returns to us the row and column they belong to in character order array
    for(int i = 0; i < 12; ++i){
        for(int j = 0; j < 10; ++j){
            if(character_order[i][j] == x){
                *row = i;
                *col = j;
                return;
            }
        }
    }
}

void init_keypad() {
	//Setting Up row pins to be in (OUTPUT)
	DDRD |= 0xF0;		//DDR for pins D4..7 set to 0
	PORTD |= 0xFF;		//Setting output to 1
	//Setting Up column pins to be INPUTS
	//DDRC &= 0x03;		//DDR for pins C4..7 set to 0
	//Setting pull up resistors for column pins
	//PORTC |= 0xFC;		//PORT for pins C4..7 set to 1
	for(int i = 0; i < 40; ++i){
	    pressed_buttons[i] = 0;
	    typed_message[i] = 0;
	}
}

void update_message(){
    for(int i = 0; i < 40; ++i){
        typed_message[i] = pressed_buttons[i];
    }
}

void clear_pressed() {
	for(int i = 0; i < 40; ++i){
	    pressed_buttons[i] = 0;
	}
}
void clear_both(){
    for(int i = 0; i < 40; ++i){
	    pressed_buttons[i] = 0;
	    typed_message[i] = 0;
	}
}
void interpret_key(char key){//when a key is typed, decide what to do with it
	if(key == 'C'){//toggle between typing number and message
		typing_number = !typing_number;
		return;
	}
	if(typing_number){
		if(key < '0' || key > '9'){//invalid phone number as numbers are between 0 and 9
			return;
		}
		else{
			num_to_send = key - '0';
		}
		return;
	}
	if(key == 'A'){// toggle to next character for key;
		if(pressed_count == 0){
			//can't change invalid
			return;
		}
		uint8_t pr,pc;
		get_pos(pressed_buttons[pressed_count - 1],&pr,&pc);
		pressed_buttons[pressed_count - 1] = character_order[pr][pc + 1];
	}
	else if(key == 'B'){//backspace
		if(pressed_count == 0){
			return;
		}
		pressed_buttons[pressed_count - 1] = 0;
		pressed_count -= 1;
	}
	else if(key == 'D'){//done writing message schedule to send it;
		update_message();
		msg_length = pressed_count;
		clear_pressed();
		pressed_count = 0;
	}
	else{
		pressed_buttons[pressed_count] = key;
		pressed_count++;
	}
}
uint8_t detect_press() {
    
	//Pressed_buttons holds the pressed buttons that have been detected
	//Pressed_count shows how long the message is
	uint8_t detected = 0;

	//Detecting Algorithm:
	//Rows need to be flipped LOW one at a time from top to bottom
	//At each moment, the columns are checked if they're still HIGH
	//Columns are low only if a button is pressed and shorts that column with the LOW row
	uint8_t row = 4;
	uint8_t column = 4;
	char key;
	while (row < 8){
		//Setting current row to output and pulling it LOW
		PORTD &= ~(1 << row);
		//Setting column to 0 for each row that is set to low
		column = 0;
		//Checking state of each column
		while (column < 4){
			if ( (PIND & (1 << column)) == 0) {
				key = KEYPAD_KEYS[row-4][column];
				interpret_key(key);
				updated = 1;
			}
			//Incrementing column
			column ++;
		}
		//Setting the row back to TRISTATE (INPUT)
		PORTD |= (1 << row);
		//Incrementing row
		row ++;
	}
	previous_key_reg_t = main_timer_now();
	return detected;
}

void check_keypad(){
    if ((main_timer_now() - previous_key_reg_t) <= 3125){
		//The next key press should be at least 1000ms after the last key press
		return;
	}
	uint8_t detected = detect_press();
	previous_key_reg_t = main_timer_now();
}

#endif		//KEYPAD
