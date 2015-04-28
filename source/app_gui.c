
#include <stdint.h>
#include <stdio.h>

#include "app_gui.h"
#include "colors.h"
#include "gui.h"
#include "drawing.h"
#include "verdanabold14.h"

#define CONFIG_TEXT_TITLE               "Bootloader"

static void header(void)
{
    drawString(10, 50, COLOR_WHITE, &verdanabold14ptFontInfo, "N Bootloader");
    drawString(10, 70, COLOR_WHITE, &verdanabold14ptFontInfo, "V: " __DATE__);
    drawLine(5, 100, 235, 100, COLOR_WHITE);
}

void initGuiModule(void) {
    
}

void appGuiInit(void) {
    gui_init();
}

void appGuiNotifyErase(void) {
   drawFill(COLOR_BLUE);
   header();
   drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "Firmware image found.");
   drawString(20, 170, COLOR_WHITE, &verdanabold14ptFontInfo, "Erasing FLASH memory...");
}

void appGuiNotifyLoading(void) {
    drawFill(COLOR_BLUE);
    header();
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "Loading image from USB...");
}

void appGuiNotifyDone(uint32_t fileSize) {
    char buff[128];

    snprintf(buff, sizeof(buff), "Loaded %d bytes.", fileSize);
    drawFill(COLOR_GREEN);
    header();
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, buff);
}

void appGuiNotifyFail00(void) {
    drawFill(COLOR_RED);
    header();
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "Failure!");
    drawString(20, 170, COLOR_WHITE, &verdanabold14ptFontInfo, "Error number: 100");
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "No firmware.");
}

void appGuiNotifyFail01(void) {
    drawFill(COLOR_RED);
    header();
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "Failure!");
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "Error number: 101");
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "Checksum mismatch.");
}

void appGuiNotifyFail02(uint32_t address) {
    char buff[128];

    snprintf(buff, sizeof(buff), "Bad address: %d.", address);

    drawFill(COLOR_RED);
    header();
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "Failure!");
    drawString(20, 150, COLOR_WHITE, &verdanabold14ptFontInfo, "Error number: 102");
    drawString(50, 70, COLOR_WHITE, &verdanabold14ptFontInfo, buff);
}
