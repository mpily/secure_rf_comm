#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include<avr/io.h>
#include"NRFLite.h"
#include"../lcd/lcd_i2c.h"
#include"../lcd/main_timer.h"
#include<stdlib.h>

const uint8_t RADIO_ID = 0;
const uint8_t PIN_RADIO_CE = 4;
const uint8_t PIN_RADIO_CSN = 4;
//uint8_t radio_init(uint8_t radioId, uint8_t cePin, uint8_t csnPin, Bitrates bitrate, uint8_t channel, uint8_t callSpiBegin);
int main(){
    init_main_timer();
    lcd_init();
    while(!radio_init(RADIO_ID, PIN_RADIO_CE,PIN_RADIO_CSN,BITRATE250KBPS,100,1)){
    	lcd_clear();
        lcd_print("can't comm");
        printDetails();
        //while(1);//wait here forever
    }
    lcd_clear();
    lcd_print("X=signal got");
    uint64_t prev_t = main_timer_now();
	while ((main_timer_now() - prev_t) < 100000);
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
        uint8_t signalStrength = scanChannel(channel,255);
        itoa(channel,chan_num,10);
        uint8_t pos = 0;
        {
            uint8_t i = 0;
            for(i = 0; i < 5 && chan_num[i] != 0; ++i){
                sol[pos++] = chan_num[i];
            }
        }
        sol[pos++] = ':';
        itoa(signalStrength,chan_num,10);
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
	    while ((main_timer_now() - prev_t) < 100000);
    }
    while(1);
    return 0;
}

