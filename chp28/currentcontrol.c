#include "currentcontrol.h"
#include "NU32.h"

void IDLE(void) {
  LATBCLR = 0xC000;
  SetPWM(0);
}

void SetPWM(int pwm) {
  OC1RS = pwm*NU32_SYS_FREQ/2000000;
}

void currentcontrol_init(void) {
  OC1CONbits.OCM = 6;
  OC1RS = 0;
  OC1R = 0;

  // timer 2
  T2CON = 0b00000;
  PR2 = NU32_SYS_FREQ/20000-1;

  // turn on
  T2CONbits.ON = 1;
  OC1CONbits.ON = 1;

  // outputs (B14 & B15)
  TRISBCLR = 0xC000;
}