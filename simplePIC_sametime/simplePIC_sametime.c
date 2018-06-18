#include <xc.h>    
      // Load the proper header for the processor

void delay(void);

int main(void) {
  TRISF = 0xFFFC;        // Pins 0 and 1 of Port F are LED1 and LED2.  Clear
                         // bits 0 and 1 to zero, for output.  Others are inputs.
  LATFbits.LATF0 = 1;    // Turn LED1 on and LED2 off.  These pins sink current
  LATFbits.LATF1 = 1;    // on the NU32, so "high" (1) = "off" and "low" (0) = "on"

  while(1) {
    delay();
    LATFINV = 0x0003;    // toggle LED1 and LED2; same as LATFINV = 0x3;
  }
  return 0;
}

void delay(void) {
  int j;
  for (j = 0; j < 4000000; j++) { // number is 4 million to actually see both going on and off at the same time
    while(!PORTDbits.RD7) {
        ;   // Pin D7 is the USER switch, low (FALSE) if pressed.
    }
  }
}

//I changed the line at LATFbits.LATF0 to 1, so LED1 blinks at the same times as LED2. Also,
// I changed the delay to 4000000 so its easier to see both LED lights turn off and on at the same time.