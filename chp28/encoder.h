#ifndef ENCODER_H
#define ENCODER_H
static int encoder_command(int read);
int encoder_ticks(void);
int encoder_angle(void);
void encoder_reset(void);
void encoder_init(void);
#endif