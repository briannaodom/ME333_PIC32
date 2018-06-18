#include "isense.h"
#include "NU32.h"         

#define SAMPLETIME 10
 
unsigned int isense_ADC(void) {
    unsigned int t = 0;
    AD1CHSbits.CH0SA = 0;
    AD1CON1bits.SAMP = 1;
    t = _CP0_GET_COUNT() + 10;
    while (_CP0_GET_COUNT() < t) {
      ;
    }
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE) {
      ;
    }
    return ADC1BUF0;
}

float isense_mA(void) {
  return (0.01171*isense_ADC()+49.2);
}

void isense_init() {
  AD1PCFGbits.PCFG0 = 0;
  AD1CON3bits.ADCS = 2;
  AD1CON1bits.ADON = 1;
}


