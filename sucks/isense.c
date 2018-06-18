#include "isense.h"
#include "NU32.h"

unsigned int isense_adc(void) {
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
  return (1.9186 * isense_adc() - 913.18);
}

void isense_init() {
  AD1PCFGbits.PCFG0 = 0;
  AD1CON3bits.ADCS = 2;
  AD1CON1bits.ADON = 1;
}
