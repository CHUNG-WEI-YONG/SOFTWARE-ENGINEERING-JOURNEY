#include <stdio.h>
int iseven(int n){
    return !(n&1);
}

int main(){
    int a=0x12345678;
    char *p=(char *)&a;

    printf(*p);

    return 0;
}