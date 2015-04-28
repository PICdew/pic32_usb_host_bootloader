
#include <xc.h>

#include "app_buzzer.h"
#include "TimeDelay.h"


void initBuzzerModule(void) {
    LATF  &= ~(0x1u << 0);
    TRISF &= ~(0x1u << 0);
}

void buzzerTone(uint32_t duration) {
    LATF  |=  (0x1u << 0);
    DelayMs(duration);
    LATF  &= ~(0x1u << 0);
}
