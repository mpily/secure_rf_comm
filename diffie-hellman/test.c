#include<stdio.h>
#include"diffie.h"
#include"../rsa/rsa.h"
int main(){
	char orig[] = {'b','i','l','l'};
	init_diffie();
	init_endequip_diffie(1,Y_i);
	uint32_t to_encrypt = 20415233;
	to_encrypt = decrypt_diffie(to_encrypt,1);
	printf("%d\n", to_encrypt);
	//read 4 bytes at a time
	
	for(uint8_t j = 0; j < 4; j++){
		to_encrypt <<= 8;
		to_encrypt |= orig[j];
		printf("%d \n", to_encrypt);
	}
	printf("what we should have -> %d\n",to_encrypt);
	//arr[0] = 0;
	//my_itoa(i,arr);
	//at this point i is 4!!!!!!!!
	//encrypt using diffie hellman first
	uint8_t msg[5];
	to_encrypt = encrypt_diffie(to_encrypt,1);
	uint32_t to_encrypt2 = to_encrypt;
	//encrypt using rsa
	printf("encrypted version -> %d\n",to_encrypt);
	for(uint8_t j = 0; j < 4; ++j){
		
		msg[4-j-1] = (to_encrypt & 0xFF);
		to_encrypt >>= 8;
	}
	for(int i = 0; i < 4; ++i){
		printf("%d\n",msg[i]);
	}
	to_encrypt = decrypt_diffie(to_encrypt2,1);
	char fin[5] = {0,0,0,0,0};
	for(int i = 3; i >= 0; --i){
		fin[i] = (to_encrypt & 0xFF);
		to_encrypt >>= 8;
	}
	printf("%s\n",fin);
	return 0;
}
