#include "NU32.h"
#include "encoder.h"
#include "isense.h"
#include "utilities.h"
#include <stdio.h>

#define BUF_SIZE 200
#define ITEST_SIZE 100

static volatile int TrajectorySize, Speed, Eint_current, Eint_position, Reference_current_array[ITEST_SIZE], Actual_current_array[ITEST_SIZE];
static volatile float Desired_angle;
static volatile float Kp_current = 20, Ki_current = 1;
static volatile float Kp_position = 150, Ki_position = 0, Kd_position = 5000;
static volatile float Desired_current = 0;

static volatile float Error_prev = 0;


static volatile int TrajectoryIndex = 0;
static volatile int TrackingTrajectory = 0;
static volatile float Trajectory[2000]; // 2000 is the maximum number of elements (10s at 200Hz)
static volatile float MeasuredTrajectory[2000]; // 2000 is the maximum number of elements (10s at 200Hz)
static volatile int StoringData = 0;    // if this flag = 1, currently storing plot data

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) CurrentController(void) {
    static int current_count = 0;
    static float actual_current = 0;
    static float reference_current = 200;

    switch (get_mode()) {
      case IDLE: {
        LATDbits.LATD5 = 0;
        OC1RS = 0;
        break;
      }
      case PWM: {
        if (Speed <= 0) {
          LATDbits.LATD5 = 0;
          OC1RS = (unsigned int) ((-Speed / 100.0) * PR3);
        } else {
          LATDbits.LATD5 = 1;
          OC1RS = (unsigned int) ((Speed / 100.0) * PR3);
        }
        break;
      }
      case ITEST: {
        // current counter changes sign of reference current every 25 iterations
        if (current_count != 0 && current_count % 25 == 0) { // check how many iterations through the ISR
          reference_current *= -1; // set reference current to opposite sign
        }
        if (current_count < 100) {
          // measure the current value
          actual_current = isense_mA();
          // store the value of reference and actual current in the reference current array
          Reference_current_array[current_count] = (int)reference_current;
          Actual_current_array[current_count] = (int)actual_current;

          // PI Controller
          float error = reference_current - actual_current;
          Eint_current = Eint_current + error;
          float u = Kp_current * error + Ki_current * Eint_current;

          float unew = ((u / 1000) / 3.3) * 100;
          if (unew > 100.0) {
            unew = 100.0;
          } else if (unew < -100.0) {
            unew = -100.0;
          }

          if (unew < 0) {
            LATDbits.LATD5 = 1;
            OC1RS = (unsigned int) ((-unew / 100.0) * PR3);
          }
          else {
            LATDbits.LATD5 = 0;
            OC1RS = (unsigned int) ((unew / 100.0) * PR3);
          }

          current_count++; // increment current count
        }
        else {
          StoringData = 0;
          current_count = 0;
          reference_current = 200;
          set_mode(IDLE);
        }

        break;
      }
      case HOLD: {
        // measure the current value
        actual_current = isense_mA();
        reference_current = Desired_current;

        // PI Controller
        float error = reference_current - actual_current;
        Eint_current = Eint_current + error;
        float u = Kp_current * error + Ki_current * Eint_current;

        float unew = ((u / 1000) / 3.3) * 100;
        if (unew > 100.0) {
          unew = 100.0;
        } else if (unew < -100.0) {
          unew = -100.0;
        }

        // set new PWM value
        if (unew < 0) {
          LATDbits.LATD5 = 1;
          OC1RS = (unsigned int) ((-unew / 100.0) * PR3);
        } else {
          LATDbits.LATD5 = 0;
          OC1RS = (unsigned int) ((unew / 100.0) * PR3);
        }

        break;
      }
      case TRACK: {
        // measure the current value
        actual_current = isense_mA();
        reference_current = Desired_current;

        // PI Controller
        float error = reference_current - actual_current;
        Eint_current = Eint_current + error;
        float u = Kp_current * error + Ki_current * Eint_current;

        float unew = ((u / 1000) / 3.3) * 100;
        if (unew > 100.0) {
          unew = 100.0;
        } else if (unew < -100.0) {
          unew = -100.0;
        }

        // set new PWM value
        if (unew < 0) {
          LATDbits.LATD5 = 1;
          OC1RS = (unsigned int) ((-unew / 100.0) * PR3);
        } else {
          LATDbits.LATD5 = 0;
          OC1RS = (unsigned int) ((unew / 100.0) * PR3);
        }

        break;
      }
    }
    IFS0bits.T2IF = 0;
}

void __ISR(_TIMER_4_VECTOR, IPL5SOFT) PositionController(void) {
    static float actual_position = 0;

    switch (get_mode()) {
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
        // measure the angle value (deg)
        actual_position = encoder_angle();

        // PI Controller
        float error = Desired_angle - actual_position;
        float edot = error - Error_prev;
        Eint_position = Eint_position + error;
        float u = -(Kp_position * error + Ki_position * Eint_position + Kd_position * edot);

        // not sure what to put here
        if (u > 300) {
          Desired_current = 300;
        } else if (u < -300) {
          Desired_current = -300;
        } else {
          Desired_current = u;
        }

        Error_prev = error;
        break;
      }
      case TRACK: {
        // measure the angle value (deg)
        actual_position = encoder_angle();

        MeasuredTrajectory[TrajectoryIndex] = actual_position;

        // PI Controller
        float error = Trajectory[TrajectoryIndex] - actual_position;
        float edot = error - Error_prev;
        Eint_position = Eint_position + error;
        float u = -(Kp_position * error + Ki_position * Eint_position + Kd_position * edot);

        // not sure what to put here
        if (u > 300) {
          Desired_current = 300;
        } else if (u < -300) {
          Desired_current = -300;
        } else {
          Desired_current = u;
        }

        Error_prev = error;

        TrajectoryIndex++;
        if (TrajectoryIndex == TrajectorySize) {
          Desired_angle = Trajectory[TrajectoryIndex-1];
          TrajectoryIndex = 0;
          TrackingTrajectory = 0;
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
  TRISDbits.TRISD5 = 0;

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

  // Init position control
  TRISDbits.TRISD6 = 0;

  // TIMER 4 200khz
  T4CONbits.TCKPS = 0b110;
  PR4 = 6249;
  TMR4 = 0;
  T4CONbits.ON = 1;
  IPC4bits.T4IP = 2;
  IPC4bits.T4IS = 1;
  IFS0bits.T4IF = 0;
  IFS0bits.T4IF = 1;
  IEC0bits.T4IE = 1;
  __builtin_enable_interrupts();

  encoder_reset(); // might need this? encoder reading high value at startup

  set_mode(IDLE);
  Eint_current = 0;
  Eint_position = 0;

  while(1)
  {
    NU32_ReadUART3(buffer,BUF_SIZE);
    NU32_LED2 = 1;
    switch (buffer[0]) {
      case 'a':
      {
        sprintf(buffer,"%d\r\n", isense_adc());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'b':
      {
        sprintf(buffer,"%f\r\n", isense_mA());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'c':
      {
        sprintf(buffer,"%d\r\n", encoder_ticks());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'd':
      {
        sprintf(buffer,"%d\r\n", encoder_angle());
        NU32_WriteUART3(buffer);
        break;
      }
      case 'e':
      {
        encoder_reset();
        break;
      }
      case 'f':                      // send PWM value to motor
      {
        set_mode(PWM);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &Speed);
        break;
      }
      case 'g':                      // set current gains
      {
        float kptemp = 0, kitemp = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &kptemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &kitemp);
        __builtin_disable_interrupts();
        Kp_current = kptemp;
        Ki_current = kitemp;
        __builtin_enable_interrupts();
        break;
      }
      case 'h':                      // get current gains
      {
        sprintf(buffer,"%f\r\n", Kp_current);
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", Ki_current);
        NU32_WriteUART3(buffer);
        break;
      }
      case 'i':                      // set position gains
      {
        float kptemp = 0, kitemp = 0, kdtemp = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &kptemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &kitemp);
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &kdtemp);
        __builtin_disable_interrupts();
        Kp_position = kptemp;
        Ki_position = kitemp;
        Kd_position = kdtemp;
        __builtin_enable_interrupts();
        break;
      }
      case 'j':                      // get position gains
      {
        sprintf(buffer,"%f\r\n", Kp_position);
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", Ki_position);
        NU32_WriteUART3(buffer);
        sprintf(buffer,"%f\r\n", Kd_position);
        NU32_WriteUART3(buffer);
        break;
      }
      case 'k':                      // current test mode
      {
        int i = 0;
        Eint_current = 0;
        StoringData = 1;
        set_mode(ITEST);
        while (StoringData) {
          ;
        }
        __builtin_disable_interrupts();
        for (; i < ITEST_SIZE; ++i) {
          sprintf(buffer,"%d %d %d\r\n", ITEST_SIZE-i, Reference_current_array[i], Actual_current_array[i]); // return Ki_current
          NU32_WriteUART3(buffer);
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'l':                      // go to motor angle
      {
        float angle = 0;
        Eint_current = 0;
        Eint_position = 0;
        Error_prev = 0;
        Desired_current = 0;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%f", &angle);
        __builtin_disable_interrupts(); // keep ISR disabled as briefly as possible
        Desired_angle = angle;
        __builtin_enable_interrupts();  // only 2 simple C commands while ISRs disabled
        set_mode(HOLD);
        break;
      }
      case 'm':
      {
        set_mode(IDLE);
        int size;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &size);
        __builtin_disable_interrupts();
        TrajectorySize = size;
        int i;
        float temp;
        for (i = 0; i < TrajectorySize; i++) {
          NU32_ReadUART3(buffer,BUF_SIZE);
          sscanf(buffer, "%f", &temp);
          Trajectory[i] = temp;
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'n':
      {
        set_mode(IDLE);
        int size;
        NU32_ReadUART3(buffer,BUF_SIZE);
        sscanf(buffer, "%d", &size);
        __builtin_disable_interrupts();
        TrajectorySize = size;
        int i;
        float temp;
        for (i = 0; i < TrajectorySize; i++) {
          NU32_ReadUART3(buffer,BUF_SIZE);
          sscanf(buffer, "%f", &temp);
          Trajectory[i] = temp;
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'o':
      {
        Eint_current = 0;
        Eint_position = 0;
        Error_prev = 0;
        Desired_current = 0;
        TrackingTrajectory = 1;
        set_mode(TRACK);
        while (TrackingTrajectory) {
          ;
        }
        set_mode(HOLD);
        __builtin_disable_interrupts();
        sprintf(buffer,"%d\r\n", TrajectorySize);
        NU32_WriteUART3(buffer);
        int i;
        __builtin_disable_interrupts();
        for (i = 0; i < TrajectorySize; ++i) {
          sprintf(buffer,"%f %f\r\n", Trajectory[i], MeasuredTrajectory[i]);
          NU32_WriteUART3(buffer);
        }
        __builtin_enable_interrupts();
        break;
      }
      case 'p':
      {
        set_mode(IDLE);
        Speed = 0;
        break;
      }
      case 'q':
      {
        // handle q for quit. Later you may want to return to IDLE mode here.
        set_mode(IDLE);
        break;
      }
      case 'r':
      {
        sprintf(buffer,"%s\r\n", get_string_mode());
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
