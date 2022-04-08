#include"../bigint/modint.h"
#include"rsa.h"
#include<stdio.h>
int main(){
    uint32_t x = 69;
    uint32_t y = encrypt(69);
    printf("%d\n",y);
    uint32_t m = decrypt(y);
    printf("%d\n",m);
    return 0;
}
