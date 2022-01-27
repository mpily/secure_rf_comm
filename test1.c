#include<avr/io.h>
#define F_CPU 8000000
#include<util/delay.h>
int main(){
    DDRB = 0xFF;
    while(1){
        int i = 0;
        PORTB ^= (1 << 0);
        _delay_ms(500);
    }
}
