
#include <xc.h>
#include <sys/attribs.h>

#include "app_buzzer.h"
#include "driver/gpio.h"
#include "driver/clock.h"

#define CONFIG_BUZZER_GPIO_PORT         &GpioB
#define CONFIG_BUZZER_GPIO_PIN          1

void initBuzzerModule(void) {
    
}

void buzzerTone(uint32_t duration) {
    
}

void __ISR(_TIMER_2_VECTOR) buzzerHandler(void) {
    
}
