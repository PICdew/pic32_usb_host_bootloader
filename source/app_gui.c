
#include <stdint.h>

#include "app_gui.h"

#define CONFIG_TEXT_TITLE               "Bootloader"


void initGuiModule(void) {
    gui_init();
}

void appGuiInit(void) {
    ;
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
