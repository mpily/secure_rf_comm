/*
    Author : mpily
    modulo integers to be used in encryption 
*/

#ifndef MODINT
#define MODINT
#include<stdint.h>
#include<stdio.h>
typedef struct {
    int32_t num;
    uint64_t mod;
}modint;
////////////////////////////////////////////////////////////
//function definitions
void from_int(uint32_t a, modint * b,uint64_t mod);//constuct from integer
void print(modint * a); // printing function
//arithmetic operations modulo mod
modint add(modint lhs, modint rhs);
modint sub(modint lhs, modint rhs);
modint mul(modint lhs, modint rhs);
modint binpow(modint b, uint64_t exp);// b ^ exp % mod
modint modinv(modint a);
modint divide(modint a, modint b);// a * b^-1 % mod
// bitwise operation
uint32_t xor_mask(modint a, uint32_t val);//a xor val
////////////////////////////////////////////////////////////

void from_int(uint32_t a, modint * b,uint64_t mod){
    b->mod = mod;
    if(a < mod){
        b-> num = a;
    }
    else{
        b->num = a % mod;
    }
}
void print(modint * a){
    printf("%d", a -> num);
}

modint add (modint lhs, modint rhs){
    uint64_t mod = lhs.mod;
    modint ans;
    uint32_t sum = lhs.num + rhs.num;
    while(sum > mod){
        sum -= mod;
    }
    from_int(sum,&ans,mod);
    return ans;
}

modint sub(modint lhs, modint rhs){ // lhs - rhs;
    uint64_t mod = lhs.mod;
    modint ans;
    uint32_t diff;
    if(lhs.num < rhs.num){
        diff = mod + lhs.num - rhs.num;
    }
    else{
        diff = lhs.num - rhs.num;
    }
    from_int(diff,&ans,mod);
    return ans;
}

modint mul(modint lhs, modint rhs){
    uint64_t mod = lhs.mod;
    modint ans;
    uint32_t prod = ((uint64_t)lhs.num * rhs.num) % mod;
    from_int(prod,&ans,mod);
    return ans;
}

modint binpow(modint b,uint64_t exp){
    uint64_t mod = b.mod;
    modint res;
    from_int(1,&res,mod);
    while(exp){
        if(exp & 1){
            res = mul(res,b);   
        }
        b = mul(b,b);
        exp >>= 1;
    }
    return res;
}

modint modinv(modint a){
    uint64_t mod = a.mod;
    modint res = binpow(a,mod - 2);
    return res;
}
modint divide(modint a, modint b){
    modint res = mul(a, modinv(b));
    return res;
}

uint32_t xor_mask(modint a, uint32_t val){
    uint32_t masked = val ^ a.num;
    return masked;
}
#endif
