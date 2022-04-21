/*
    Used to implement Diffie Hellman Key Exchange
    Step 1. Generate X_i
    Step 2. Compute Y_i
    Step 3. Store X_i and Y_i
    Step 4. When other device sends Y_theirs, Compute key = Y_theirs ^ X_i
    Step 5. Compute modulo inverse of key.
    Step 6. Encryption is Modulo multiplication
    Step 7. Decryption is Multiplying by modulo inverses
*/
#ifndef DIFFIE
#define DIFFIE
#include"../bigint/modint.h"
//keys and mods and generator
const uint64_t mod = 2147483647;
const uint32_t generator = 16807;
uint32_t encryption_keys_diffie[128];
uint32_t decryption_keys_diffie[128];
/////personal Y_i and X_i
const uint32_t X_i = 31489;
uint32_t Y_i;
//////////////////functions///////////////////////
// initializing personal keys;
void init_diffie(){
    modint a;
    from_int(generator,&a,mod);
    modint b = binpow(a,X_i);
    Y_i = b.num;
}
//when a device sends its Y_i initialize communication with them
void init_endequip_diffie(uint8_t num,uint32_t Y_theirs){
    modint a;
    from_int(Y_theirs,&a,mod);
    modint b = binpow(a,X_i);
    encryption_keys_diffie[num] = b.num;
    a = modinv(b);
    decryption_keys_diffie[num] = a.num;
}
//encrypt packet. Uses modulo multiplication
uint32_t encrypt_diffie(uint32_t data,uint8_t num){
    modint a,b;
    from_int(data,&a,mod);
    from_int(encryption_keys_diffie[num],&b,mod);
    modint d = mul(a,b);
    return d.num;
}
//decrypt packet. Uses modulo multiplication by modulo inverse
uint32_t decrypt_diffie(uint32_t data, uint8_t num){
    modint a,b;
    from_int(data,&a,mod);
    from_int(decryption_keys_diffie[num],&b,mod);
    modint d = mul(a,b);
    return d.num;
}
#endif
