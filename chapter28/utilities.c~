#include "utilities.h"

static volatile Mode mode = IDLE;

int GetMode(void) {
  return mode;
}


void SetMode(Mode m) {
  __builtin_disable_interrupts();
  mode = m;
  __builtin_enable_interrupts();
}
