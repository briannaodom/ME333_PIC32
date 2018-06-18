#include "isense.h"
#include <xc.h>

unsigned int isense_read(void){
  unsigned int t;
  AD1CHSbits.CH0SA = 11;
  AD1CON1bits.SAMP = 1;
  t = _CP0_GET_COUNT() + 10;
  while (_CP0_GET_COUNT() < t);
  AD1CON1bits.SAMP = 0;
  while (!AD1CON1bits.DONE);
  return (ADC1BUF0);
}

void isense_init(void) {
  // ADC
  AD1PCFG = 0xF7FF;
  AD1CON3bits.ADCS = 3;
  AD1CON1bits.ADON = 1;

  // timer
  INTCONbits.INT0EP = 0;          // step 3: INT0 triggers on falling edge
  IPC0bits.INT0IP = 2;            // step 4: interrupt priority 2
  IPC0bits.INT0IS = 1;            // step 4: interrupt priority 1
  IFS0bits.INT0IF = 0;            // step 5: clear the int flag
  IEC0bits.INT0IE = 1;            // step 6: enable INT0 by setting IEC0<3>
}
