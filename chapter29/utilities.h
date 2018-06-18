
#ifndef UTILITIES__H__
#define UTILITIES__H__

typedef enum {IDLE, PWM, ITEST, HOLD, TRACK} Mode;

int GetMode();
char * get_mode_str();
void SetMode(Mode m);

#endif
