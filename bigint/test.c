#include"modint.h"
int main(){
    modint a , b;
    from_int(13736289,&a);
    from_int(83477473,&b);
    modint c = mul(a,b);
    print(&c);
    return 0;
}
