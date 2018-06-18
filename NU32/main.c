#include <stdio.h>
#include "helper.h"           


int main(void) {


  char output[100];
  NU32_Startup();
  sprintf(output, "\nThe Program Started Nicely\n");
  NU32_WriteUART3(output);

  Investment inv;                

  while(getUserInput(&inv)) {    
    inv.invarray[0] = inv.inv0;  
    calculateGrowth(&inv);       
    sendOutput(inv.invarray,     
              inv.years);       
  }
  return 0;                      
} 