#include "NU32.h"          // constants, funcs for startup and UART
#include "LCD.h"
#include <stdio.h>

static volatile int state = 0;

void __ISR(_EXTERNAL_0_VECTOR, IPL2SOFT) Ext0ISR(void) { // step 1: the ISR
  unsigned int time;
  static unsigned int time1, time2;
  char message[50] = {};

  // wait 10 ms
  time = _CP0_GET_COUNT() +4000000;
  while(_CP0_GET_COUNT() < time){;}


  //if button is 0 
  if(PORTDbits.RD7 == 0){
    if(state == 0){
      time1 = _CP0_GET_COUNT();
      LCD_Clear(); // clear LCD screen
      LCD_Move(0,0);
      LCD_WriteString("Press USER butt-");
      LCD_Move(1,0);
      LCD_WriteString("on to stop!");
      state = 1;
    }
    else if(state == 1){
      time2 = _CP0_GET_COUNT();
      sprintf( message, "%d secs elapsed", (time2-time1)/40000000);
      LCD_Clear(); // clear LCD screen
      LCD_Move(0,0);
      LCD_WriteString(message);
      LCD_Move(1,0);
      LCD_WriteString("Press to start");
      state = 0;
    }
  }
  IFS0bits.INT0IF = 0; 
}

int main(void) {
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
  LCD_Setup();
  LCD_Clear(); // clear LCD screen
  LCD_Move(0,0);
  LCD_WriteString("Started");
  LCD_Move(1,0);
  LCD_WriteString("Press button");

  __builtin_disable_interrupts(); // step 2: disable interrupts
  INTCONbits.INT0EP = 0;          // step 3: INT0 triggers on falling edge
  IPC0bits.INT0IP = 2;            // step 4: interrupt priority 2
  IPC0bits.INT0IS = 1;            // step 4: interrupt priority 1
  IFS0bits.INT0IF = 0;            // step 5: clear the int flag
  IEC0bits.INT0IE = 1;            // step 6: enable INT0 by setting IEC0<3>
  __builtin_enable_interrupts();  // step 7: enable interrupts
                                  // Connect RD7 (USER button) to INT0 (RD0)
  while(1) {
      ; // do nothing, loop forever
  }

  LCD_Clear(); // clear LCD screen
  LCD_Move(0,0);

  return 0;
}