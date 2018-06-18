#include "NU32.h"          // config bits, constants, funcs for startup and UART
// include other header files here
#include "encoder.h"
#define BUF_SIZE 200

int main() 
{
  int var = 1;
  char buffer[BUF_SIZE];
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
  NU32_LED1 = 1;  // turn off the LEDs
  NU32_LED2 = 1;        
  __builtin_disable_interrupts();
  // in future, initialize modules or peripherals here  
  encoder_init();
  __builtin_enable_interrupts();
  
 
  while(var)
  {
    NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
    NU32_LED2 = 1;                   // clear the error LED
    switch (buffer[0]) {
      case 'a':
      {
        sprintf(buffer, "%d", isense_adc());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'b':
      {
        sprintf(bugger, "%d", isense_mA());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'c':
      {
        sprintf(buffer, "%d", encoder_counts()); 
        NU32_WriteUART3(buffer);
        break;
      }
      case 'd':                      // dummy command for demonstration purposes
      {
        sprintf(buffer, "%f", (float)(encoder_counts()/32768.0)*360.0); 
        NU32_WriteUART3(buffer);
      break;
      }
      case 'e':
      {
        encoder_command(0);
      break;
      }
      case 'f':
      {
        NU32_ReadUART3(buffer, BUF_SIZE);
        SetPWM(atoi(buffer));
        SetMode(1);
      break;
      }
       case 'g':
      {
        break;
      }
      case 'h':
      {
        break;
      }
      case 'i':
      {
        break;
      }
      case 'j':
      {
        break;
      }
      case 'k':
      {
        break;
      }
      case 'l':
      {
        break;
      }
      case 'm':
      {
        break;
      }
      case 'n':
      {
        break;
      }
      case 'o':
      {
        break;
      }
      case 'p':
      {
        SetMode(0);
        IDLE(); //lowercase?
      break;
      }
      case 'q':
      {
        var = 0;     // handle q for quit. Later you may want to return to IDLE mode here. 
        IDLE();
        cont_loop = 0
        break;
      }
      case 'r':
      {
        sprintf(buffer,"%d"GetMode());
        NU32_WriteUART3(buffer); 
      break;
      }

      default:
      {
        NU32_LED2 = 0;  // turn on LED2 to indicate an error
        break;
      }
    }
  }
  return 0;
}
