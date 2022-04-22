#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include<avr/io.h>
#include"nrf/NRFLite.h"
#include"lcd/lcd_i2c.h"
#include"lcd/main_timer.h"
#include"rsa/rsa.h"
#include"keypad/keypad.h"
#include"diffie-hellman/diffie.h"
#include<stdlib.h>
const uint8_t RADIO_ID = 1;
const uint8_t PIN_RADIO_CE = 3;
const uint8_t PIN_RADIO_CSN = 4;
uint8_t DESTINATION_RADIO_ID = 0;

/////////////////////////
//uint8_t radio_init(uint8_t radioId, uint8_t cePin, uint8_t csnPin, Bitrates bitrate, uint8_t channel, uint8_t callSpiBegin);
typedef enum{INITIALIZE, CHANNEL_CONTROL, E2EENCRYPTINIT, E2EMSG}RadioPacketType;
typedef struct{
    RadioPacketType packetType;
	uint8_t intended_final_destination;
	uint8_t from_radio_id;
	uint32_t n;//private key
	uint32_t e;//private key
}InitPacket;//structure of initialization packet
typedef struct{
    RadioPacketType packetType;
    uint8_t intended_final_destination;
    uint8_t from_radio_id;
    uint8_t msg[20];
}ReceivedPacket;
typedef struct{
    RadioPacketType packetType;
    uint8_t intended_final_destination;
    uint8_t from_radio_id;
    uint8_t msg[20];
}EndUserMsg;
ReceivedPacket new_packet;//a newly received packet
char received_msg[40];//to store received messages
uint8_t received_id;//store the id of received message
uint8_t recently_received;
uint8_t await_channel_info = 0;
uint8_t await_diffie_info = 0;
///////////////////debugging functions /////////////////////////
/*void my_itoa(uint32_t num, char * arr){
    for(int i = 0; i < 10; ++i){
        arr[i] = 0;
    }
    int pos = 0;
    if(num == 0){
        arr[pos] = '0';
    }
    while(num){
        arr[pos] = '0' + (num % 10);
        num/= 10;
        pos ++;
    }
    pos --;
    int i = 0;
    while(i < pos){
        char tmp = arr[i];
        arr[i] = arr[pos];
        arr[pos] = tmp;
        pos --;
        i ++;
    }
    lcd_clear();
    lcd_print("--");
    lcd_print(arr);
    _delay_ms(500);
}*/
////////////////////////////////////////////////////////////
uint8_t send_init_message(){
    //lcd_clear();
    //lcd_print("sending init message");
    EndUserMsg initmessage;
    initmessage.packetType = INITIALIZE;
    initmessage.intended_final_destination = 0;
    initmessage.from_radio_id = RADIO_ID;
    uint32_t n = encrypt_rsa(mod_rsa_me,mod_bs_rsa,encrypt_bs_rsa);
    uint32_t e = encrypt_rsa(encrypt_key_rsa,mod_bs_rsa,encrypt_bs_rsa);
    int8_t i = 0;
    for(;i < 4; ++i){
    	initmessage.msg[4 - i - 1] = (n & 0xFF);
    	n >>= 8;
    } 
    i = 0;
    for(; i < 4; ++i){
    	initmessage.msg[8 - i - 1] = (e & 0xFF);
    	e >>= 8;
    }
    if (send(DESTINATION_RADIO_ID, &initmessage, sizeof(initmessage),REQUIRE_ACK)) // 'send' puts the radio into Tx mode.
    {
        lcd_clear();
        lcd_print("...Success");
        return 1;//successfully initialized
    }
    else
    {
        lcd_clear();
        lcd_print("...Failed");
        return 0;//failed to initialize;
        
    }
    await_channel_info = 1;
    return 0;
}
void handle_channel_control(){//when a channel control packet is received
 	uint32_t channel_info = 0;//will be 4 bytes long and is encrypted
    for(int i = 0; i < 4; ++i){
    	channel_info <<= 8;
    	channel_info |= new_packet.msg[i];
    }
    uint8_t comm_channel = decrypt_rsa(channel_info);
    char num[10];
    //my_itoa(comm_channel,num);
    switch_channels(comm_channel);
}
void handle_e2e_init(){//when end to end encryption initialization packet is received
	/*
		Packet structure : 
			-4 bytes and they encode Y_theirs
			-1 byte whether they want a response
	*/
	uint8_t their_id = new_packet.from_radio_id;
	//lcd_print("initing: ");
	char dbg_id[2] = {'0' + their_id,0};
	lcd_print(dbg_id);
	uint32_t Y_theirs = 0;
	for(int i = 0; i < 4; ++i){
		Y_theirs <<= 8;
		Y_theirs |= new_packet.msg[i];
	}
	Y_theirs = decrypt_rsa(Y_theirs);
	init_endequip_diffie(their_id,Y_theirs);
	if(new_packet.msg[4]){
		EndUserMsg init_msg;
		init_msg.packetType =  E2EENCRYPTINIT;
		init_msg.intended_final_destination = their_id;
		init_msg.from_radio_id = RADIO_ID;
		uint32_t Y_send = encrypt_rsa(Y_i,mod_bs_rsa,encrypt_bs_rsa);
		for(int i = 3; i >= 0; --i){
			init_msg.msg[i] = (Y_send & 0xFF);
			Y_send >>= 8;
		}
		init_msg.msg[4] = 0;
		for(int i = 5; i < 20; ++i)init_msg.msg[i] = 0;
		send(DESTINATION_RADIO_ID, &init_msg, sizeof(init_msg),REQUIRE_ACK);
	}
	//lcd_clear();
	//lcd_print("done initing");
	//_delay_ms(500);
}
void handle_e2e_msg(){
	char arr[15];
	for(int i = 0; i < 15; ++i)arr[i] = 0;
	received_id = new_packet.from_radio_id;
	uint8_t i = 0;
	while(i < 20){
		uint32_t bytes = 0;
		for(int j = 0; j < 4; ++j,i++){
			bytes <<= 8;
			bytes |= new_packet.msg[i];
		}
		bytes = decrypt_rsa(bytes);
		bytes = decrypt_diffie(bytes,received_id);
		for(int j = 0; j < 4; ++j){
			uint8_t nxt_char = (bytes & 0xFFUL);
			received_msg[i - j - 1] = nxt_char;
			bytes >>= 8;
		}
	}
	received_msg[20] = 0;
	//recently_received = 1;
	lcd_clear();
	lcd_print("from : ");
	char tgt[3] = {'0' + received_id,0,0};
	lcd_print(tgt);
	lcd_second_line();
	lcd_print(received_msg);
	_delay_ms(1000);
}
void checkRadio(){
    while(hasData(0)){
        readData(&new_packet);
        if(new_packet.packetType == CHANNEL_CONTROL){
           handle_channel_control();
        }
        else if(new_packet.packetType == E2EENCRYPTINIT){
        	handle_e2e_init();
        }
        else if(new_packet.packetType == E2EMSG){
        	handle_e2e_msg();//has a lot of control stuff best to handle alone;	
        }
    }
}
void init_e2e(uint8_t num){
	//lcd_clear();
	//lcd_print("sending e2e again");
	EndUserMsg init_msg;
	init_msg.packetType =  E2EENCRYPTINIT;
	init_msg.intended_final_destination = num;
	init_msg.from_radio_id = RADIO_ID;
	uint32_t Y_send = encrypt_rsa(Y_i,mod_bs_rsa,encrypt_bs_rsa);
	//break down the encrypted message and repackage it into byte sized segments
	
	for(int i = 3; i >= 0; --i){
		init_msg.msg[i] = (Y_send & 0xFFUL);
		Y_send >>= 8;
	}
	if(encryption_keys_diffie[num] == 0)init_msg.msg[4] = 1;
	else init_msg.msg[4] = 0;
	for(int i = 5; i < 20; ++i)init_msg.msg[i] = 0;
	send(DESTINATION_RADIO_ID, &init_msg, sizeof(init_msg),REQUIRE_ACK);
	while(encryption_keys_diffie[num] == 0){
		checkRadio();
		//lcd_clear();
		//lcd_print("not yet");
		send(DESTINATION_RADIO_ID, &init_msg, sizeof(init_msg),REQUIRE_ACK);
		_delay_ms(10);//wait some time then try again
	}
	//lcd_clear();
	//lcd_print("success --> ");
	//char numstr[2] = {'0' + num, 0};
	//lcd_print(numstr);
	//_delay_ms(500);
}

void send_e2e_msg(){
	EndUserMsg msg_packet;
	msg_packet.packetType = E2EMSG;
	msg_packet.intended_final_destination = num_to_send;
	msg_packet.from_radio_id = RADIO_ID;
	init_e2e(num_to_send);
	msg_length += (4 - (msg_length % 4));//make message length divisible by 4
	//lcd_clear();
	//lcd_print("done init encrypt");
	//_delay_ms(1000);
	//char arr[20];
	//for(int i = 0; i < 20; ++i) arr[i] = 0;
	uint8_t i = 0;
	while(i < msg_length){
		uint32_t to_encrypt = 0;
		//read 4 bytes at a time
		for(uint8_t j = 0; j < 4; j++,i++){
			to_encrypt <<= 8;
			to_encrypt |= typed_message[i];
		}
		//arr[0] = 0;
		//my_itoa(i,arr);
		//at this point i is 4!!!!!!!!
		//encrypt using diffie hellman first
		to_encrypt = encrypt_diffie(to_encrypt,num_to_send);
		//encrypt using rsa
		to_encrypt = encrypt_rsa(to_encrypt,mod_bs_rsa,encrypt_bs_rsa);
		//break down the encrypted message and repackage it into byte sized segments
		for(uint8_t j = 0; j < 4; ++j){
			msg_packet.msg[i-j-1] = (to_encrypt & 0xFFUL);
			to_encrypt >>= 8;
		}
		//arr[0] = 0;
		//my_itoa(i,arr);
	}
	for(;i < 20; ++i)msg_packet.msg[i] = 0;
	msg_length = 0;
	checkRadio();
	while(!send(DESTINATION_RADIO_ID, &msg_packet, sizeof(msg_packet),REQUIRE_ACK)){
		lcd_clear();
		lcd_print("message not yet sent");
		_delay_ms(10);
	}
	lcd_clear();
	lcd_print("message sent");
	checkRadio();//place in receive mode;
	_delay_ms(20);
}
void display_on_lcd(){
	if(recently_received){
		lcd_clear();
		lcd_print(received_msg);
		_delay_ms(1000);
		recently_received = 0;
	}
	else if(updated && pressed_count){
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
		lcd_second_line();
		lcd_print("typing sth");
	}
}

int main(){
    init_main_timer();
    lcd_init();
    init_keypad();
    init_diffie();
    init_endequip_diffie(RADIO_ID,Y_i);
    while(!radio_init(RADIO_ID, PIN_RADIO_CE,PIN_RADIO_CSN,BITRATE1MBPS,100,1)){
    	lcd_clear();
        lcd_print("can't comm");
        //printDetails();
        //while(1);//wait here forever
    }
    while(!send_init_message()){
        _delay_ms(10);//wait for some time then try again.
    }
    while(1){
    	if(!await_channel_info && !await_diffie_info){
    		check_keypad();
    	}
    	if(msg_length){
    		send_e2e_msg();
    	}
    	checkRadio();
    	display_on_lcd();
    }
    return 0;
}
