/*
    author : mpily
    big integer library to be used for encryption.
*/

#ifndef BIGINTS
#define BIGINTS
#include<stdint.h>
#include<stdio.h>
#include<stdlib>
typedef struct {
    uint8_t num[10];// 80 bits
} bigint;
// function definitions
////////////////////////////////////////////////////////////////////////////////////////////
//initialisation
void gen_bigint(int s, bigint * number);// make a bigint from a regular integer
//bit operations
uint8_t get_bit(bigint* number, uint8_t pos); //get a particular bit
void set_bit(bigint * number, uint8_t pos,uint8_t val);// set a specific bit
bigint big_xor(bigint * lhs, bigint * rhs);// returns lhs ^ rhs
//printing
void print_bigint(bigint* number); // print a bigint in hex
//comparison
uint8_t less(bigint * lhs, bigint * rhs); // return 1 if lhs < rhs
uint8_t greater(bigint * lhs, bigint * rhs);
uint8_t equal(bigint * lhs, bigint * rhs);
//arithmetic
bigint add(bigint lhs, bigint rhs);// add 2 bigints
bigint mul(bigint rhs, bigint lhs); // multiply 2 bigints
bigint sub(bigint rhs, bigint lhs); // subtract 2 bigints make sure that rhs >= lhs as there are no negative numbers
bigint div(bigint rhs, bigint lhs); // returns floor(lhs/ rhs)
bigint mod(bigint lhs, bigint rhs);// returns lhs modulo rhs
///////////////////////////////////////////////////////////////////////////////////////////
uint8_t get_bit(bigint * number, uint8_t pos){
    uint8_t arr = 4 - pos/8;
    uint8_t bit = pos % 8;
    return (number-> num[arr] & (1 << bit)) > 0;
}
void set_bit(bigint * number, uint8_t pos,uint8_t val){
    uint8_t arr = 4 - pos/8;
    uint8_t bit = pos % 8;
    if(val)
        number -> num[arr] |= (1 << bit);
    else
        number -> num[arr] &= (~(1 << bit));
}
bigint add(bigint * lhs, bigint * rhs){
    bigint ans;
    uint8_t carry = 0;
    uint8_t pos = 4;
    for(int bit = 0; bit < 32; ++bit){
        if(bit && bit % 8 == 0){
            pos --;
        }
        uint8_t l = get_bit(lhs,bit);
        uint8_t r = get_bit(rhs,bit);
        if(l ^ r ^ carry){
            set_bit(ans,bit,1);
        }
        else{
            set_bit(ans,bit,0);
        }
    }
    return ans;
}
#endif
