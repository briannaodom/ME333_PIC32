#include "utilities.h"

static volatile Mode mode = IDLE;

int GetMode(void) {
  return mode;
}

char * get_mode_str(void) {
  switch(mode) {
    case IDLE: {
      return "IDLE";
      break;
    }
    case PWM: {
      return "PWM";
      break;
    }
    case ITEST: {
      return "ITEST";
      break;
    }
    case HOLD: {
      return "HOLD";
      break;
    }
    case TRACK: {
      return "TRACK";
      break;
    }
  }
}

void SetMode(Mode m) {
  __builtin_disable_interrupts();
  mode = m;
  __builtin_enable_interrupts();
}
