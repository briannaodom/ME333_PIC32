#ifndef UTILITIES_H
#define UTILITIES_H
enum Mode {IDLE, PWM};
static volatile int mode;
int GetMode(void);
void SetMode(int newMode);
void utilities_init(void);
#endif