#include<stdio.h>

void switch_eg(long x, long n, long *dest){
    long val = x;
    switch(n){
        case 0:
            val *=13;
            break;
        case 2:
            val += 10;
        case 3:
            val += 11;
            break;
        case 4:
        case 6:
            val +=11;
            break;
        default:
            val = 0;
    }
    *dest = val;
}
int main(){
    long x = 1;
    long n = 1;
    long c = 3;
    switch_eg(x, n, &c);
    return 0;
}