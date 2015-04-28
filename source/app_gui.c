
#include <stdint.h>

#include "app_gui.h"
#include "colors.h"
#include "gui.h"
#include "drawing.h"
#include "verdanabold14.h"

#define CONFIG_TEXT_TITLE               "Bootloader"


void initGuiModule(void) {
    
}

void appGuiInit(void) {
    gui_init();
}

void appGuiNotifyErase(void) {
   drawFill(COLOR_BLUE);
}

void appGuiNotifyLoading(void) {
    drawFill(COLOR_BLUE);
}

void appGuiNotifyDone(uint32_t fileSize) {
    (void)fileSize;
    
    drawFill(COLOR_GREEN);
}

void appGuiNotifyFail00(void) {
    drawFill(COLOR_RED);
}

void appGuiNotifyFail01(void) {
    drawFill(COLOR_RED);
}

void appGuiNotifyFail02(uint32_t address) {
    (void)address;

    drawFill(COLOR_RED);
}
