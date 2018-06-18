#include "NU32.h"          // config bits, constants, funcs for startup and UART
// include other header files here
#include "encoder.h"
#include <stdio.h>
#include "utilities.h"
#include "positioncontrol.h"
#include "isense.h"
#include "currentcontrol.h"

#define BUF_SIZE 200
#define ITEST_SIZE 100

int loop = 1;
static volatile int actual_current_array[ITEST_SIZE];
static volatile float desired_angle;
static volatile float desired_current = 0;
static volatile int eint_current = 0;
static volatile int eint_position = 0;
static volatile float error_prev = 0;
static volatile float Kpcurrent = 20, Kicurrent = 1; 
static volatile float Kpposition = 150, Kiposition = 0, Kdposition = 5000; 
static volatile float MeasuredTraj[2000]; 
static volatile int reference_current_array[ITEST_SIZE];
static volatile int speed;
static volatile int StoringData = 0;
static volatile int TrajSize;
static volatile int TrajIndex = 0;
static volatile int TrackingTraj = 0;
static volatile float Traj[2000]; 

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) CurrentController(void) {
    static int current_count = 0;
    static float actual_current = 0;
    static float reference_current = 200;

    switch (GetMode()) {
      case IDLE: {
        LATDbits.LATD6 = 0;
        OC1RS = 0;
        break;
      }
      case PWM: {
        if (speed <= 0) {
          LATDbits.LATD6 = 0;
          OC1RS = (unsigned int)((-speed/100.0)*3999);
        } else {
          LATDbits.LATD6 = 1;
          OC1RS = (unsigned int)((speed/100.0)*PR3);
        }
        break;
      }
      case ITEST: {
   
        if (current_count != 0 && current_count % 25 == 0) { 
          reference_current *= -1; 
        }
        if (current_count < 100) {

          actual_current = isense_mA();

          reference_current_array[current_count] = (int)reference_current;
          actual_current_array[current_count] = (int)actual_current;
          
          float error = reference_current - actual_current;
          eint_current = eint_current + error;
          float u = Kpcurrent * error + Kicurrent * eint_current;


          float unew = ((u / 1000) / 3.3) * 100;
          if (unew > 100.0) {
            unew = 100.0;
          } else if (unew < -100.0) {
            unew = -100.0;
          }

          if (unew < 0) {
            LATDbits.LATD6 = 1;
            OC1RS = (unsigned int) ((-unew / 100.0) * PR3);
          } else {
            LATDbits.LATD6 = 0;
            OC1RS = (unsigned int) ((unew / 100.0) * PR3);
          }

          current_count++; 
        }
        else {
          StoringData = 0;
          current_count = 0;
          reference_current = 200;
          SetMode(IDLE);
        }

        break;
      }
      case HOLD: {
       
        actual_current = isense_mA();
        reference_current = desired_current;
        float error = reference_current - actual_current;
        eint_current = eint_current + error;
        float u = Kpcurrent * error + Kicurrent * eint_current;

        float unew = ((u / 1000) / 3.3) * 100;
        if (unew > 100.0) {
          unew = 100.0;
        } else if (unew < -100.0) {
          unew = -100.0;
        }

        // set new PWM value
        if (unew < 0) {
          LATDbits.LATD6 = 1;
          OC1RS = (unsigned int) ((-unew / 100.0) * PR3);
        } else {
          LATDbits.LATD6 = 0;
          OC1RS = (unsigned int) ((unew / 100.0) * PR3);
        }

        break;
      }
      case TRACK: {
        actual_current = isense_mA();
        reference_current = desired_current;
        float error = reference_current - actual_current;
        eint_current = eint_current + error;
        float u = Kpcurrent * error + Kicurrent * eint_current;

        float unew = ((u / 1000) / 3.3) * 100;
        if (unew > 100.0) {
          unew = 100.0;
        } else if (unew < -100.0) {
          unew = -100.0;
        }

        if (unew < 0) {
          LATDbits.LATD6 = 1;
          OC1RS = (unsigned int) ((-unew / 100.0) * PR3);
        } else {
          LATDbits.LATD6 = 0;
          OC1RS = (unsigned int) ((unew / 100.0) * PR3);
        }

        break;
      }
    }
    IFS0bits.T2IF = 0;
}

void __ISR(_TIMER_4_VECTOR, IPL5SOFT) PositionController(void) {
    static float actual_position = 0;

    switch (GetMode()) {
      case IDLE: {

        break;
      }
      case PWM: {

        break;
      }
      case ITEST: {

        break;
      }
      case HOLD: {

        actual_position = encoder_angle();


        float error = desired_angle - actual_position;
        float edot = error - error_prev;
        eint_position = eint_position + error;
        float u = -(Kpposition * error + Kiposition * eint_position + Kdposition * edot);

  
        if (u > 300) {
          desired_current = 300;
        } else if (u < -300) {
          desired_current = -300;
        } else {
          desired_current = u;
        }

        error_prev = error;
        break;
      }
      case TRACK: {
        actual_position = encoder_angle();

        MeasuredTraj[TrajIndex] = actual_position;

        float error = Traj[TrajIndex] - actual_position;
        float edot = error - error_prev;
        eint_position = eint_position + error;
        float u = -(Kpposition * error + Kiposition * eint_position + Kdposition * edot);

        if (u > 300) {
          desired_current = 300;
        } else if (u < -300) {
          desired_current = -300;
        } else {
          desired_current = u;
        }

        error_prev = error;

        TrajIndex++;
        if (TrajIndex == TrajSize) {
          desired_angle = Traj[TrajIndex-1];
          TrajIndex = 0;
          TrackingTraj = 0;
        }
        break;
      }
    }
    IFS0bits.T4IF = 0;
}

int main() 
{
  char buffer[BUF_SIZE];
  NU32_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
  NU32_LED1 = 1;  // turn off the LEDs
  NU32_LED2 = 1;   

 isense_init();
 
  __builtin_disable_interrupts();
  encoder_init();

  // Init current control
  TRISDbits.TRISD6 = 0;

  // TIMER 2 5khz
  T2CONbits.TCKPS = 0b000;
  PR2 = 15999;
  TMR2 = 0;
  T2CONbits.ON = 1;
  IPC2bits.T2IP = 5;
  IPC2bits.T2IS = 0;
  IFS0bits.T2IF = 0;
  IFS0bits.T2IF = 1;
  IEC0bits.T2IE = 1;

  // TIMER 3
  T3CONbits.TCKPS = 0b000;
  PR3 = 3999;
  TMR3 = 0;
  OC1CONbits.OCTSEL = 1;
  OC1CONbits.OCM = 0b110;
  OC1RS = 4000;
  OC1R = 4000;
  T3CONbits.ON = 1;
  OC1CONbits.ON = 1;

  // Init position control
  TRISDbits.TRISD6 = 0;

  // TIMER 4 200khz
  T4CONbits.TCKPS = 0b110;
  PR4 = 6249;
  TMR4 = 0;
  T4CONbits.ON = 1;
  IPC4bits.T4IP = 5;
  IPC4bits.T4IS = 0;
  IFS0bits.T4IF = 0;
  IFS0bits.T4IF = 1;
  IEC0bits.T4IE = 1;
  __builtin_enable_interrupts();

  encoder_reset(); // might need this? encoder reading high value at startup

 SetMode(IDLE);
 eint_current = 0;
 eint_position = 0;
 float Kptemp = 0; 
 float Kitemp = 0;
 float Kdtemp = 0;
 float angle = 0;
  
  while(loop)
  {
    NU32_ReadUART3(buffer,BUF_SIZE); // we expect the next character to be a menu command
    NU32_LED2 = 1;                   // clear the error LED
    switch (buffer[0]) {
      case 'a':
      {
        // read current count
       sprintf(buffer,"%d\n", isense_ADC());
       NU32_WriteUART3(buffer);
        break;
      }
      case 'b':
      {
        // read current mA
        sprintf(buffer,"%f\n", isense_mA());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'c':
      {
        // read encoder counts
        sprintf(buffer,"%d\n", encoder_ticks());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'd':
      {
        //read encoder angle
        sprintf(buffer,"%d\n", encoder_angle());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'e':
      {
        //reset encoder
       sprintf(buffer,"%d\n", encoder_reset());
       NU32_WriteUART3(buffer);
        break;
      }
      case 'f':
      {
        // Set PWM
        SetMode(PWM);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", speed);
        break;
      }
      case 'g':
      //setting current gains
      {
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kptemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kitemp);
        __builtin_disable_interrupts(); 
        Kpcurrent = Kptemp;
        Kicurrent = Kitemp;
        __builtin_enable_interrupts();  
        break;
      }
      case 'h':
      //getting current gains
      {
        sprintf(buffer,"%f\r\n", Kpcurrent); 
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", Kicurrent); 
        NU32_WriteUART3(buffer);

        break;
      }
      case 'i':
      //setting position gains
      {
         NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kptemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kitemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kdtemp);
        
        __builtin_disable_interrupts(); 
        Kpposition = Kptemp;
        Kiposition = Kitemp;
        Kdposition = Kdtemp;
        __builtin_enable_interrupts(); 
        break;
      }
      case 'j':
      //getting position gains
      {
        sprintf(buffer,"%f\r\n", Kpposition); 
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", Kiposition);
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", Kdposition); 
        NU32_WriteUART3(buffer);
        break;
      }
      case 'k':
      {
        // test current control
        eint_current = 0;
        StoringData = 1;
        SetMode(ITEST);
        while (StoringData) {
          ;
        }
        sprintf(buffer,"%d\r\n", ITEST_SIZE);
        NU32_WriteUART3(buffer);
        int j;
        
        __builtin_disable_interrupts();
        for (j = 0; j < ITEST_SIZE; j++) {
          sprintf(buffer,"%d %d\r\n", reference_current_array[j], actual_current_array[j]); 
          NU32_WriteUART3(buffer);
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'l':
      //go to motor angle
      {
       eint_current = 0;
       eint_position = 0;
       error_prev = 0;
       desired_current = 0;
       NU32_ReadUART3(buffer,BUF_SIZE);
       sscanf(buffer, "%f", &angle);
       
       __builtin_disable_interrupts(); // keep ISR disabled as briefly as possible
       desired_angle = angle;
       __builtin_enable_interrupts();  // only 2 simple C commands while ISRs disabled
       SetMode(HOLD);
        break;
      }
      case 'm':
      {
        // load step trajectory
        SetMode(IDLE);
        int size;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &size);
        
        __builtin_disable_interrupts();
        TrajSize = size;
        
        int j;
        float temp;
        for (j = 0; j < TrajSize; j++) {
          NU32_ReadUART3(buffer,BUF_SIZE);
          sscanf(buffer, "%f", &temp);
          Traj[j] = temp;
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'n':
      {
        // load cubic trajectory
        SetMode(IDLE);
        int size;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &size);
        __builtin_disable_interrupts();
        TrajSize = size;
        int i;
        float temp;
        for (i = 0; i < TrajSize; i++) {
          NU32_ReadUART3(buffer,BUF_SIZE);
          sscanf(buffer, "%f", &temp);
          Traj[i] = temp;
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'o':
      {
        // execute trajectory

        break;
      }
      case 'p':
      {
        // Set mode to idle.
        SetMode(IDLE);
        speed = 0;
        break;

      }
      case 'q':
      {
        
        SetMode(IDLE);
        break;
      }
      case 'r':
      {
        // Get Mode
        sprintf(buffer,"%d\n", GetMode());
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


