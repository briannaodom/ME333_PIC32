#ifndef ENCODER_H
#define ENCODER_H
static int encoder_command(int read);
void encoder_init(void);
int encoder_ticks(void);
int encoder_angle(void);
int encoder_reset(void);

#endif
