#include <xc.h>    
#define MAXCYCLES 4000000
#define DELTACYCLES 100000
      // Load the proper header for the processor

void delay(int cycles);

int main(void) {
  int i;
  int cycles;

  TRISF = 0xFFFC;        // Pins 0 and 1 of Port F are LED1 and LED2.  Clear
                         // bits 0 and 1 to zero, for output.  Others are inputs.
  LATFbits.LATF0 = 0;    // Turn LED1 on and LED2 off.  These pins sink current
  LATFbits.LATF1 = 1;    // on the NU32, so "high" (1) = "off" and "low" (0) = "on"

  while(1) {
    for(i = 0; i <= 40; i++){
        cycles = MAXCYCLES-i*DELTACYCLES;
        delay(cycles);
        LATFINV = 0x0003;    // toggle LED1 and LED2; same as LATFINV = 0x3;
  }
}
  return 0;
}

void delay(int cycles) {
  int j;
  for (j = 0; j < cycles; j++) { // cycles decreases by deltacycles each incrementation
    while(!PORTDbits.RD7) {
        ;   // Pin D7 is the USER switch, low (FALSE) if pressed.
    }
  }
}
