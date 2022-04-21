#include"../bigint/modint.h"
#include"rsa.h"
#include<stdio.h>
void to_uint8(uint32_t source, uint8_t * store)
{
  for(int idx = 0; idx < 4; idx++)
  {
  	uint8_t tmp = (source & 0xFF);
  	printf("%d\n",tmp);
    store[3 - idx] = (source & 0xFF);
    source >>= 8;
  }
}
int main(){
    /*uint64_t x = encrypt_rsa(1228139371ULL,mod_rsa_me,encrypt_key_rsa);
    printf("%lld\n -> encrypted \n",x);
    uint64_t mod = mod_rsa_me;
    uint64_t pow = decipher_key_rsa;
    uint64_t ans = 1;
    uint64_t tst = (5ULL * 2573128253ULL) % 4288547088ULL;
    printf("%lld\n check inverses \n", tst);
    while(pow){
    	if(pow & 1){
    		ans = 1ULL * ans * x;
    		ans %= mod;
    	}
    	x *= x;
    	x %= mod;
    	pow >>= 1;
    }
	printf("%lld\n",ans);*/
	uint8_t arr[10];
	to_uint8(4220781301ULL,arr);
	for(int i = 0; i < 4; ++i){
		printf("%d\n",arr[i]);
	}
    return 0;
}
