#include <stdio.h>     
#include "NU32.h"
#define MAX_YEARS 100   


typedef struct {
  double inv0;                   
  double growth;                  
  int years;                      
  double invarray[MAX_YEARS+1];  
} Investment;              


int getUserInput(Investment *invp);    
void calculateGrowth(Investment *invp); 
void sendOutput(double *arr, int years);


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


void calculateGrowth(Investment *invp) {

 int i;


 for (i = 1; i <= invp->years; i= i + 1) {   
                                              
     invp->invarray[i] = invp->growth * invp->invarray[i-1]; 
   }
 } 

int getUserInput(Investment *invp) {

  int valid;     
  char valid1[100];  
  char output[100];
  char maxyears[100];
  char inv1[100];

  sprintf(maxyears, "\r\nEnter investment, growth rate, number of yrs (up to %d): \r\n", MAX_YEARS);
  NU32_WriteUART3(maxyears);

  NU32_ReadUART3(inv1, 100);
  sscanf(inv1, "\r\n%f %f %d\r\n", &(invp->inv0) , &(invp->growth), &(invp->years));

  sprintf(output,"\r\nInput:%f %f %d\r\n", (invp->inv0), (invp->growth), (invp->years));

  NU32_WriteUART3(output);
  
  
  valid = ((invp->inv0) > 0) && ((invp->growth) > 0) &&
  ((invp->years) > 0) && ((invp->years) <= MAX_YEARS);

  sprintf(valid1, "\r\nValid input?  %d\r\n", valid);
  NU32_WriteUART3(valid1);


  if (!valid) { 
    sprintf(output, "\r\nInvalid input; exiting.\r\n");
    NU32_WriteUART3(output);
  }
  return(valid);
} 



void sendOutput(double *arr, int yrs) {

  int i;
  char outstring[100];  
  char output[100];    

  sprintf(output, "\r\nRESULTS:\r\n\n");
  NU32_WriteUART3(output);
  for (i=0; i<=yrs; i++) {  
    sprintf(outstring,"\r\nYear %3d:  %10.2f\r\n",i,arr[i]); 
    NU32_WriteUART3(outstring);
    sprintf(outstring, "%s");
  
  }
  sprintf(output,"\n");
  NU32_WriteUART3(output);
} 
