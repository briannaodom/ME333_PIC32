#include "NU32.h"          // constants, functions for startup and UART

int main(void) {
  NU32_Startup();          // cache on, interrupts on, LED/button init, UART init

  T3CONbits.TCKPS = 0;     // Timer3 prescaler N=1 (1:1)
  PR3 = 3999;              // period = (PR2+1) * N * ns = .05 ms, 20 kHz
  TMR3 = 0;                // initial TMR3 count is 0
  OC1CONbits.OCM = 0b110;
  OC1CONbits.OCTSEL = 1;   // PWM mode without fault pin; other OC1CON bits are defaults
  OC1RS = 3000;             // duty cycle = OC1RS/(PR2+1) = 25%
  OC1R = 3000;              // initialize before turning OC1 on; afterward it is read-only
  T3CONbits.ON = 1;        // turn on Timer3
  OC1CONbits.ON = 1;       // turn on OC1

  //_CP0_SET_COUNT(0);       // delay 4 seconds to see the 25% duty cycle on a 'scope
  //while(_CP0_GET_COUNT() < 4 * 40000000) {
  //  ;
  //}
  //OC1RS = ;            // set duty cycle to 75%
  while(1) {
    ;                      // infinite loop
  }
  return 0;
}

