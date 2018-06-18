#include <stdio.h>     
#include <math.h>
#include "NU32.h"

int main(){

char output[100];
float f1=2.07, f2;
long double d1=2.07, d2;

unsigned int elapsed, elapsed1, elapsed2, elapsed3, initial, final;

_CP0_SET_COUNT(0);
f2 = cosf(f1);
final = _CP0_GET_COUNT();
elapsed = final - initial;

_CP0_SET_COUNT(0);
f2 = sqrtf(f1);
final = _CP0_GET_COUNT();
elapsed1 = final - initial;

_CP0_SET_COUNT(0);
d2 = cos(d1);
final = _CP0_GET_COUNT();
elapsed2 = final - initial;

_CP0_SET_COUNT(0);
d2 = sqrt(d1);
final = _CP0_GET_COUNT();
elapsed3 = final - initial;

sprintf(output,"%d, %d, %d, %d \r\n", elapsed, elapsed1, elapsed2, elapsed3);
NU32_WriteUART3(output);

return 0;

}