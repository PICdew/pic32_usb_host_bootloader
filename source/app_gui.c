
#include <stdint.h>

#include "app_gui.h"

#define CONFIG_TEXT_TITLE               "Bootloader"


static void gpuInitEarly(void) {
    /* Values specific to QVGA LCD display */
    
}

static void gpuInitLate(void) {
    /* Do a power cycle for safer side */
    
}

void initGuiModule(void) {
    gpuInitEarly();
}

void appGuiInit(void) {
    gpuInitLate();
}

void appGuiNotifyErase(void) {
   
}

void appGuiNotifyLoading(void) {
    
}

void appGuiNotifyDone(uint32_t fileSize) {
    
}

void appGuiNotifyFail00(void) {
    
}

void appGuiNotifyFail01(void) {
    
}

void appGuiNotifyFail02(uint32_t address) {
    
}
