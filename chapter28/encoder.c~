#include "encoder.h"
#include "NU32.h"
static int encoder_command(int read) {                                        
                                       
   SPI4BUF = read;                    
   while (!SPI4STATbits.SPIRBF) { ; }   
   SPI4BUF;                             
   SPI4BUF = 5;                        
   while (!SPI4STATbits.SPIRBF) { ; }
   return SPI4BUF;
 }


void encoder_init(void) {   
  SPI4CON = 0;           
  SPI4BUF;                  
  SPI4BRG = 0x4;            
  SPI4STATbits.SPIROV = 0;  
  SPI4CONbits.MSTEN = 1;    
  SPI4CONbits.MSSEN = 1;     
  SPI4CONbits.MODE16 = 1;    
  SPI4CONbits.MODE32 = 0;   
  SPI4CONbits.SMP = 1;       
  SPI4CONbits.ON = 1;       
}

int encoder_ticks(void) {
  encoder_command(1);
  return encoder_command(1);
}

int encoder_angle(void) {
  return ((float)encoder_ticks()-32768.0)*360.0/(448.0*4.0);
}

int encoder_reset(void) {
  encoder_command(0);
  encoder_command(0);
  return encoder_command(0);
}

