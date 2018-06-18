#ifndef CALCULATE__H__
#define CALCULATE__H__
#define MAX_YEARS 100 

#include <xc.h>                     // processor SFR definitions
#include <sys/attribs.h>            // __ISR macro

#define NU32_LED1 LATFbits.LATF0    // LED1 on the NU32 board
#define NU32_LED2 LATFbits.LATF1    // LED2 on the NU32 board
#define NU32_USER PORTDbits.RD7     // USER button on the NU32 board
#define NU32_SYS_FREQ 80000000ul    // 80 million Hz

typedef struct {
  double inv0;                   
  double growth;                  
  int years;                      
  double invarray[MAX_YEARS+1];  
} Investment;   

void NU32_Startup(void);
void NU32_ReadUART3(char * string, int maxLength);
void NU32_WriteUART3(const char * string);
int getUserInput(Investment *invp);    
void calculateGrowth(Investment *invp); 
void sendOutput(double *arr, int years);

#endif // CALCULATE__H__
