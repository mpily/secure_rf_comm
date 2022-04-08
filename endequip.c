#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include<avr/io.h>
#include"nrf/NRFLite.h"
#include"lcd/lcd_i2c.h"
#include"lcd/main_timer.h"
#include"rsa/rsa.h"
#include<stdlib.h>
uint8_t channels[150];
const uint8_t RADIO_ID = 1;
const uint8_t PIN_RADIO_CE = 3;
const uint8_t PIN_RADIO_CSN = 4;
uint8_t DESTINATION_RADIO_ID = 0;
/*
encryption keys for base station
*/
const uint32_t mod_bs = 238080883ULL;
const uint32_t encrypt_bs = 3;
/*
    encryption keys unique to device
    //must satisfy 
    r = 11503
    s = 12967
    n = r.s = 149159401
    d = 7
    
*/
const uint32_t mod = 149159401ULL;
const uint32_t decipher_key = 63914971ULL;
const uint32_t encrypt_key = 7;
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
    uint8_t msg[20];
}ReceivedPacket;
typedef struct{
    RadioPacketType packetType;
    uint8_t intended_final_destination;
    uint8_t msg[20];
}end_user_msg;

uint8_t send_init_message(){
    lcd_clear();
    lcd_print("sending init message");
    InitPacket initmessage;
    initmessage.packetType = INITIALIZE;
    initmessage.intended_final_destination = 0;
    initmessage.from_radio_id = RADIO_ID;
    initmessage.n = encrypt(mod,mod_bs,encrypt_bs);
    initmessage.e = encrypt(encrypt_key,mod_bs,encrypt_bs);
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
    return 0;
}
int main(){
    init_main_timer();
    lcd_init();
    while(!radio_init(RADIO_ID, PIN_RADIO_CE,PIN_RADIO_CSN,BITRATE1MBPS,100,1)){
    	lcd_clear();
        lcd_print("can't comm");
        printDetails();
        //while(1);//wait here forever
    }
    while(!send_init_message()){
        _delay_ms(10);//wait for some time then try again.
    }
    return 0;
}
