#include "NU32.h"          
#include "encoder.h"
#include "isense.h"
#include "utilities.h"
#include "currentcontrol.h"
#include "positioncontrol.h"
#include <stdio.h>

#define BUF_SIZE 200
#define ITEST_SIZE 100

static volatile int speed;
static volatile float desired_angle;
static volatile float Kpcurrent = 20, Kicurrent = 1; 
static volatile float Kp_pos = 150, Ki_pos = 0, Kd_pos = 5000; 
static volatile float desired_current = 0;
static volatile int eint_current = 0;
static volatile int eint_pos = 0;
static volatile float error_prev = 0;
static volatile int reference_current_array[ITEST_SIZE];
static volatile int actual_current_array[ITEST_SIZE];
static volatile int TrajSize;
static volatile int TrajIndex = 0;
static volatile int TrackingTraj = 0;
static volatile float Traj[2000]; 
static volatile float measuredTraj[2000]; 
static volatile int storingData = 0;   

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) CurrentController(void) {
    static int currentCount = 0;
    static float actualCurrent = 0;
    static float refCurrent = 200;

    switch (GetMode()) {
      case IDLE: {
        LATDbits.LATD5 = 0;
        OC1RS = 0;
        break;
      }
      case PWM: {
        if (speed <= 0) {
          LATDbits.LATD5 = 0;
          OC1RS = (unsigned int) ((-speed/100.0) * PR3);
        } else {
          LATDbits.LATD5 = 1;
          OC1RS = (unsigned int) ((speed/100.0) * PR3);
        }
        break;
      }
      case ITEST: {
  
        if (currentCount != 0 && currentCount % 25 == 0) {
             refCurrent = refCurrent *= -1; 
        }
        if (currentCount < 100) {
          actualCurrent = mA_isense();
          reference_current_array[currentCount] = (int)refCurrent;
          actual_current_array[currentCount] = (int)actualCurrent;
        
          
          float error = refCurrent - actualCurrent;
          eint_current = eint_current + error;
          float u = Kpcurrent * error + Kicurrent * eint_current;
          float unew = ((u/1000)/3.3)*100;
          if (unew > 100.0) {
            unew = 100.0;
          } 
          else if (unew < -100.0) {
            unew = -100.0;
          }

          if (unew < 0) {
            LATDbits.LATD5 = 1;
            OC1RS = (unsigned int) ((-unew/100.0)*PR3);
          } else {
            LATDbits.LATD5 = 0;
            OC1RS = (unsigned int) ((unew/100.0)*PR3);
          }

          currentCount++; 
        }
        else {
          storingData = 0;
          currentCount = 0;
          refCurrent = 200;
          SetMode(IDLE);
        }

        break;
      }
      case HOLD: {
        
        actualCurrent = mA_isense();
        refCurrent = desired_current;

        float error = refCurrent - actualCurrent;
        eint_current = eint_current + error;
        float u = Kpcurrent * error + Kicurrent * eint_current;

        float unew = ((u/1000)/3.3)*100;
        if (unew > 100.0) {
          unew = 100.0;
        }
         else if (unew < -100.0) {
          unew = -100.0;
        }

        if (unew < 0) {
          LATDbits.LATD5 = 1;
          OC1RS = (unsigned int) ((-unew/100.0)*PR3);
        } else {
          LATDbits.LATD5 = 0;
          OC1RS = (unsigned int) ((unew/100.0)*PR3);
        }

        break;
      }
      case TRACK: {
        actualCurrent = mA_isense();
        refCurrent = desired_current;

        float error = refCurrent - actualCurrent;
        eint_current = eint_current + error;
        float u = Kpcurrent * error + Kicurrent * eint_current;

        float unew = ((u/1000)/3.3)*100;
        if (unew > 100.0) {
          unew = 100.0;
        } 
        else if (unew < -100.0) {
          unew = -100.0;
        }

        if (unew < 0) {
          LATDbits.LATD5 = 1;
          OC1RS = (unsigned int) ((-unew/100.0)*PR3);
        } 
        else {
          LATDbits.LATD5 = 0;
          OC1RS = (unsigned int) ((unew/100.0)*PR3);
        }

        break;
      }
    }
    IFS0bits.T2IF = 0;
}

void __ISR(_TIMER_4_VECTOR, IPL5SOFT) PositionController(void) {
    static float actual_pos = 0;

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
       
        actual_pos = encoder_angle();
        float error = desired_angle - actual_pos;
        float ederiv = error - error_prev;
        eint_pos = eint_pos + error;
        float u = -(Kp_pos * error + Ki_pos * eint_pos + Kd_pos * ederiv);

   
        if (u > 300) {
          desired_current = 300;
        } 
        else if (u < -300) {
          desired_current = -300;
        } 
        else {
          desired_current = u;
        }
        error_prev = error;
        break;
      }
      case TRACK: {
        actual_pos = encoder_angle();
        measuredTraj[TrajIndex] = actual_pos;

   
        float error = Traj[TrajIndex] - actual_pos;
        float ederiv = error - error_prev;
        eint_pos = eint_pos + error;
        float u = -(Kp_pos * error + Ki_pos * eint_pos + Kd_pos * ederiv);

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
  NU32_Startup();
  NU32_LED1 = 1; 
  NU32_LED2 = 1;

  isense_init();
  
  __builtin_disable_interrupts();
  encoder_init();
  currentcontrol_init();
  positioncontrol_init();
  __builtin_enable_interrupts();

  //encoder_reset();
  SetMode(IDLE);

  float Kptemp = 0;
  float Kitemp = 0; 
  float Kdtemp = 0;
  float angle = 0;

  while(1)
  {
    NU32_ReadUART3(buffer,BUF_SIZE); 
    NU32_LED2 = 1;                   
    switch (buffer[0]) {
      case 'a':                     
      {
        sprintf(buffer,"%d\r\n", ADC_isense()); 
        // ADC count
        NU32_WriteUART3(buffer);
        break;
      }
      case 'b':                      
      // read ADC (mA)
      {
        sprintf(buffer,"%f\r\n", mA_isense()); 
        NU32_WriteUART3(buffer);
        break;
      }
      case 'c':                      
      // read encoder (counts)
      {
        sprintf(buffer,"%d\r\n", encoder_ticks()); 
        NU32_WriteUART3(buffer);
        break;
      }
      case 'd':                      
      // read encoder (degs)
      {
        sprintf(buffer,"%d\r\n", encoder_angle()); 
        NU32_WriteUART3(buffer);
        break;
      }
      case 'e':     
      // reset encoder                
      {
        encoder_reset();
        sprintf(buffer,"%d\r\n", encoder_angle()); 
        break;
      }
      case 'f':      
      // set PWM               
      {
        SetMode(PWM);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &speed);
        break;
      }
      case 'g':                     
       // set current gains
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
      {                  
      // get current gains
        sprintf(buffer,"%f\r\n", Kpcurrent); 
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", Kicurrent); 
        NU32_WriteUART3(buffer);
        break;
      }
      case 'i':                      
      // set position gains
      {
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kptemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kitemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &Kdtemp);
        
        __builtin_disable_interrupts(); 
        Kp_pos = Kptemp;
        Ki_pos = Kitemp;
        Kd_pos = Kdtemp;
        __builtin_enable_interrupts(); 
       
        break;
      }
      case 'j':                      
      // get position gains
      {
       sprintf(buffer,"%f\r\n", Kp_pos); 
       NU32_WriteUART3(buffer);
       sprintf(buffer,"%f\r\n", Ki_pos);
       NU32_WriteUART3(buffer);
       sprintf(buffer,"%f\r\n", Kd_pos); 
       NU32_WriteUART3(buffer);
        break;
      }
      case 'k':                      
      // current test mode
       {
        int i = 0;
        eint_current = 0;
        storingData = 1;
        SetMode(ITEST);
        while (storingData) {
         ;
        }
        __builtin_disable_interrupts();
        for (; i < ITEST_SIZE; ++i) {
          sprintf(buffer,"%d %d %d\r\n", ITEST_SIZE-i, reference_current_array[i], actual_current_array[i]);
          NU32_WriteUART3(buffer);
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'l':                      
      // go to motor angle
      {
       eint_current = 0;
       eint_pos = 0;
       error_prev = 0;
       desired_current = 0;
       NU32_ReadUART3(buffer,BUF_SIZE);
       sscanf(buffer, "%f", &angle);
       
       __builtin_disable_interrupts(); 
       desired_angle = angle;
       __builtin_enable_interrupts(); 
       SetMode(HOLD);
        break;
      }
      case 'm':
      {
      //load step trajectory
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
      //load cubic trajectory
      {
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
      //execute trajectory
       eint_current = 0;
       eint_pos = 0;
       error_prev = 0;
       desired_current = 0;
       TrackingTraj = 1;
       SetMode(TRACK);
       while (TrackingTraj) {
          ;
        }
        SetMode(HOLD);
        
        __builtin_disable_interrupts();
        sprintf(buffer,"%d\r\n", TrajSize);
        NU32_WriteUART3(buffer);
        int j;
        __builtin_disable_interrupts();
        for (j = 0; j < TrajSize; j++) {
          sprintf(buffer,"%f %f\r\n", Traj[j], measuredTraj[j]);
          NU32_WriteUART3(buffer);
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'p':
      {
      //set mode to idle
        SetMode(IDLE);
        speed = 0;
        break;
      }
      case 'q':
      {
      //quit commands, set to IDLE
        SetMode(IDLE);
        break;
      }
      case 'r':
      {
        // Get Mode
        sprintf(buffer,"%s\r\n", get_mode_str());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'z':
      {
        sprintf(buffer,"%d\r\n", TrajSize);
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%d\r\n", TrajIndex);
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", Traj[200]);
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", measuredTraj[200]);
        NU32_WriteUART3(buffer);
        break;
      }
      default:
      {
        NU32_LED2 = 0;  
        break;
      }
    }
  }
  return 0;
}
