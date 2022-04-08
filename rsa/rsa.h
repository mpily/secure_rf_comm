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
uint32_t encrypt(uint32_t data,uint32_t n, uint32_t e){
    modint a;
    from_int(data,&a,n);
    modint d = binpow(a,e);
    //return 328509;
    return d.num;
}
uint32_t decrypt(uint32_t data){
    modint a;
    from_int(data,&a,238080883ULL);
    modint m = binpow(a,158699659ULL);
    return m.num;
}
#endif
