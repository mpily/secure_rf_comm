/*
    used to implement rsa algorithm for encryption
    two primes being used
    r = 12809 s = 18587
    n = r.s = 238080883
    e = 3
    d = modinv 3 mod 238049488 = 158699659
    Encryption
    C = M ^ e
    Decryption = C ^ d
*/
#ifndef RSA
#define RSA
#include"../bigint/modint.h"
/*
encryption keys for base station
*/
const uint32_t mod_bs_rsa = 4292870399ULL;
const uint32_t encrypt_bs_rsa = 11;
/*
    encryption keys unique to device
    //must satisfy 
    //////////////////set 1 - Base Station/////////////////////////
    r = 65521
    s = 65519
    n = r.s = 4292870399
    d = 11
    e = 1560996131
    
    ////////////////set 2 - UE //////////////////////////
    r = 65497
    s = 65479
    n = r.s = 4288678063
    d = 5
    e = 2573128253
    
    //////////////set 3 - UE2 //////////////////////////////
    r = 
*/
const uint32_t mod_rsa_me = 4288678063ULL;
const uint32_t decipher_key_rsa = 2573128253ULL;
const uint32_t encrypt_key_rsa = 5;
uint32_t encrypt_rsa(uint32_t data,uint32_t n, uint32_t e){
    modint a;
    from_int(data,&a,n);
    modint d = binpow(a,e);
    //return 328509;
    return d.num;
}
uint32_t decrypt_rsa(uint32_t data){
    modint a;
    from_int(data,&a,mod_rsa_me);
    modint m = binpow(a,decipher_key_rsa);
    return m.num;
}
#endif
