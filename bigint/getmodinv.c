#include<stdio.h>
const int mod = 239270451;
int extended_euclid(int a, int b, int * x, int * y){//get gcd using extended euclid algorithm
    if (a == 0){
        (*x) = 0;(*y) = 1;
        return b;
    }
    int x_tmp,y_tmp;
    int g = extended_euclid(b % a, a, &x_tmp, &y_tmp);
    *x = y_tmp - (b / a) * x_tmp;
    *y = x_tmp;
 
    return g;
}
int main(){
    int m = 149134932UL;
    int x, y;
    int g = extended_euclid(7,149134932UL,&x,&y);
    if(g != 1){
        
        printf("there is no modulo inverse %d \n", g);
    }
    else{
        x = (x % m + m) % m;
        printf("modulo inverse is %d", x);
    }
}
