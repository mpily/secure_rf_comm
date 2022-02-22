#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include<avr/io.h>
#include"NRFLite.h"
#include"../lcd/lcd_i2c.h"
#include"../lcd/main_timer.h"
#include<stdlib.h>
uint8_t channels[150];
const uint8_t RADIO_ID = 0;
const uint8_t PIN_RADIO_CE = 3;
const uint8_t PIN_RADIO_CSN = 4;
uint8_t DESTINATION_RADIO_ID = 1;
//uint8_t radio_init(uint8_t radioId, uint8_t cePin, uint8_t csnPin, Bitrates bitrate, uint8_t channel, uint8_t callSpiBegin);
uint64_t last_send_time = 0;
typedef enum  {HELLO}RadioPacketType;
typedef struct{
    RadioPacketType PacketType;
    uint8_t FromRadioId;
    uint32_t theData;
}RadioPacket;
void scan_channel_test(){
    lcd_clear();
    lcd_print("X=signal got");
    uint64_t prev_t = main_timer_now();
	while ((main_timer_now() - prev_t) < 1000);
    uint8_t channel = 0;
    char sol[20];
    char chan_num[5] = {0,0,0,0,0};
    {
        uint8_t i = 0;
        for(i = 0; i < 20; ++i){
            sol[i] = 0;
        }
    }
    for(channel = 0; channel <= MAX_NRF_CHANNEL; channel++){
        channels[channel] = scanChannel(channel,255);
    }
    for(channel = 0; channel <= MAX_NRF_CHANNEL;channel++){
    	uint8_t pos = 0;
    	itoa(channel,chan_num,10);
        {
            uint8_t i = 0;
            for(i = 0; i < 5 && chan_num[i] != 0; ++i){
                sol[pos++] = chan_num[i];
            }
        }
        sol[pos++] = ':';
        itoa(channels[channel],chan_num,10);
        {
            uint8_t i = 0;
            for(i = 0; i < 5 && chan_num[i] != 0; ++i){
                sol[pos++] = chan_num[i];
            }
        }
        sol[pos] = 'x';
        lcd_clear();
        lcd_print(sol);
        uint64_t prev_t = main_timer_now();
	    while ((main_timer_now() - prev_t) < 1000);
    }
    printDetails();
    while(1);
}
void checkRadio(){
	lcd_clear();
	lcd_print("receiving hello");
    while(hasData(0)){
        RadioPacket new_packet;
        readData(&new_packet);
        if(new_packet.PacketType == HELLO){
            char rec_data[10];
            {
                itoa(new_packet.theData,rec_data,10);
                lcd_clear();
                lcd_print(rec_data);
            }
        }
    }
}
void sendHello(){
    lcd_clear();
    lcd_print("sending hello");
    RadioPacket radioData;
    radioData.PacketType = HELLO;
    radioData.FromRadioId = RADIO_ID;
    radioData.theData = 69;

    if (send(DESTINATION_RADIO_ID, &radioData, sizeof(radioData),REQUIRE_ACK)) // 'send' puts the radio into Tx mode.
    {
        lcd_clear();
        lcd_print("...Success");
    }
    else
    {
        lcd_clear();
        lcd_print("...Failed");
    }
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
    //printDetails();
    //while(1);
    while(1){
	    if((main_timer_now() - last_send_time) > 10000){
	    	last_send_time = main_timer_now();
	    	sendHello();
	    }
	    checkRadio();
    }
    //scan_channel_test();
    return 0;
}

