
#include <stdint.h>

#include "FT_Platform.h"
#include "app_gui.h"

/**@name        Display coefficients
 */
#define DISP_WIDTH                      320
#define DISP_HEIGHT                     240
#define DISP_H_CYCLE                    408
#define DISP_H_OFFSET                   70
#define DISP_H_SYNC0                    0
#define DISP_H_SYNC1                    10
#define DISP_V_CYCLE                    263
#define DISP_V_OFFSET                   13
#define DISP_V_SYNC0                    0
#define DISP_V_SYNC1                    2
#define DISP_PCLK                       8
#define DISP_SWIZZLE                    2
#define DISP_PCLK_POL                   0

#define CONFIG_POS_V_TITLE              40
#define CONFIG_POS_H_TITLE              DISP_WIDTH / 2
#define CONFIG_OPTION_TITLE             OPT_CENTER
#define CONFIG_POS_V_INFO               200
#define CONFIG_POS_H_INFO               DISP_WIDTH / 2
#define CONFIG_FONT_SIZE                29

#define CONFIG_TEXT_TITLE               "Bootloader"

static Ft_Gpu_Hal_Context_t Gpu;
static struct spiHandle     GpuSpi;

static void gpuInitEarly(void) {
    /* Values specific to QVGA LCD display */
    Ft_Gpu_HalInit_t halinit;                       /* Not used in this port */

    Gpu.hal_handle = &GpuSpi;
    Ft_Gpu_Hal_Init(&halinit);
    Ft_Gpu_Hal_Open(&Gpu);
}

static void gpuInitLate(void) {
    /* Do a power cycle for safer side */
    Ft_Gpu_Hal_Powercycle(&Gpu, FT_TRUE);
    //Ft_Gpu_Hal_Rd16(&Gpu, RAM_G);

    /* Access address 0 to wake up the FT800 */
    Ft_Gpu_HostCommand(&Gpu, FT_GPU_ACTIVE_M);
    Ft_Gpu_Hal_Sleep(40);

    /* Set the clk to external clock */
    Ft_Gpu_HostCommand(&Gpu, FT_GPU_EXTERNAL_OSC);
    Ft_Gpu_Hal_Sleep(20);

    /* Switch PLL output to 48MHz */
    Ft_Gpu_HostCommand(&Gpu, FT_GPU_PLL_48M);
    Ft_Gpu_Hal_Sleep(20);

    /* Do a core reset for safer side */
    Ft_Gpu_HostCommand(&Gpu, FT_GPU_CORE_RESET);
    Ft_Gpu_Hal_Sleep(40);
    
    Ft_Gpu_Hal_Wr16(&Gpu, REG_HCYCLE,   DISP_H_CYCLE);
    Ft_Gpu_Hal_Wr16(&Gpu, REG_HOFFSET,  DISP_H_OFFSET);
    Ft_Gpu_Hal_Wr16(&Gpu, REG_HSYNC0,   DISP_H_SYNC0);
    Ft_Gpu_Hal_Wr16(&Gpu, REG_HSYNC1,   DISP_H_SYNC1);
    Ft_Gpu_Hal_Wr16(&Gpu, REG_VCYCLE,   DISP_V_CYCLE);
    Ft_Gpu_Hal_Wr16(&Gpu, REG_VOFFSET,  DISP_V_OFFSET);
    Ft_Gpu_Hal_Wr16(&Gpu, REG_VSYNC0,   DISP_V_SYNC0);
    Ft_Gpu_Hal_Wr16(&Gpu, REG_VSYNC1,   DISP_V_SYNC1);
    Ft_Gpu_Hal_Wr8(&Gpu,  REG_SWIZZLE,  DISP_SWIZZLE);
    Ft_Gpu_Hal_Wr8(&Gpu,  REG_PCLK_POL, DISP_PCLK_POL);

    Ft_Gpu_Hal_Wr16(&Gpu, REG_HSIZE,    DISP_WIDTH);
    Ft_Gpu_Hal_Wr16(&Gpu, REG_VSIZE,    DISP_HEIGHT);

    Ft_Gpu_Hal_Wr8(&Gpu,  REG_GPIO_DIR, 0x83 | Ft_Gpu_Hal_Rd8(&Gpu, REG_GPIO_DIR));
    Ft_Gpu_Hal_Wr8(&Gpu,  REG_GPIO,     0x83 | Ft_Gpu_Hal_Rd8(&Gpu, REG_GPIO));

    /* Touch configuration - configure the resistance value to 1200 - this value is specific to customer requirement and derived by experiment */
    Ft_Gpu_Hal_Wr16(&Gpu, REG_TOUCH_RZTHRESH,1200);
    Ft_Gpu_Hal_Wr32(&Gpu, RAM_DL + 0,   CLEAR_COLOR_RGB(0, 0, 0));//set the background to black
    Ft_Gpu_Hal_Wr32(&Gpu, RAM_DL + 4,   CLEAR(1,1,1));
    Ft_Gpu_Hal_Wr32(&Gpu, RAM_DL + 8,   DISPLAY());
    Ft_Gpu_Hal_Wr8(&Gpu,  REG_DLSWAP,   DLSWAP_FRAME);
    Ft_Gpu_Hal_Wr8(&Gpu,  REG_PCLK,     DISP_PCLK);                             /* After this display is visible on the LCD                 */
}

static void title(void) {

}

void initGuiModule(void) {
    gpuInitEarly();
}

void appGuiInit(void) {
    gpuInitLate();
}

void appGuiNotifyErase(void) {
    Ft_Gpu_CoCmd_Dlstart(&Gpu);
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(32, 32, 32));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(8, 8, 128));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, 80));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, DISP_HEIGHT));
    Ft_Gpu_Hal_WrCmd32(&Gpu,COLOR_RGB(255, 255, 255));
    Ft_Gpu_CoCmd_Text(&Gpu, CONFIG_POS_H_TITLE, CONFIG_POS_V_TITLE, CONFIG_FONT_SIZE, OPT_CENTER, CONFIG_TEXT_TITLE);
    Ft_Gpu_CoCmd_Text(&Gpu, CONFIG_POS_H_INFO,  CONFIG_POS_V_INFO,      CONFIG_FONT_SIZE, OPT_CENTER, "Loading new firmware");
    Ft_Gpu_CoCmd_Spinner(&Gpu, DISP_WIDTH / 2, DISP_HEIGHT / 2, 0, 0);
    Ft_Gpu_Hal_WrCmd32(&Gpu, DISPLAY());
    Ft_Gpu_CoCmd_Swap(&Gpu);
    Ft_Gpu_Hal_WaitCmdfifo_empty(&Gpu);
}

void appGuiNotifyLoading(void) {
#if 0
    Ft_Gpu_CoCmd_Dlstart(&Gpu);
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(32, 32, 32));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(8, 8, 128));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, 80));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, DISP_HEIGHT));
    Ft_Gpu_Hal_WrCmd32(&Gpu,COLOR_RGB(255, 255, 255));
    Ft_Gpu_CoCmd_Text(&Gpu, DISP_WIDTH / 2, CONFIG_POS_V_TITLE, CONFIG_FONT_SIZE, OPT_CENTER, CONFIG_TEXT_TITLE);
    Ft_Gpu_CoCmd_Text(&Gpu, DISP_WIDTH / 2, CONFIG_POS_V_INFO,       CONFIG_FONT_SIZE, OPT_CENTER, "Loading new image file");
    Ft_Gpu_CoCmd_Spinner(&Gpu, DISP_WIDTH / 2, DISP_HEIGHT / 2, 0, 0);
    Ft_Gpu_Hal_WrCmd32(&Gpu, DISPLAY());
    Ft_Gpu_CoCmd_Swap(&Gpu);
    Ft_Gpu_Hal_WaitCmdfifo_empty(&Gpu);
#endif
}

void appGuiNotifyDone(void) {
    Ft_Gpu_CoCmd_Dlstart(&Gpu);
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(4, 92, 4));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(8, 8, 128));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, 80));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, DISP_HEIGHT));
    Ft_Gpu_Hal_WrCmd32(&Gpu,COLOR_RGB(255, 255, 255));
    Ft_Gpu_CoCmd_Text(&Gpu, CONFIG_POS_H_TITLE, CONFIG_POS_V_TITLE, CONFIG_FONT_SIZE, OPT_CENTER, CONFIG_TEXT_TITLE);
    Ft_Gpu_CoCmd_Text(&Gpu, CONFIG_POS_H_INFO,  CONFIG_POS_V_INFO,       CONFIG_FONT_SIZE, OPT_CENTER, "Firmware loaded");
    Ft_Gpu_Hal_WrCmd32(&Gpu, DISPLAY());
    Ft_Gpu_CoCmd_Swap(&Gpu);
    Ft_Gpu_Hal_WaitCmdfifo_empty(&Gpu);
}

void appGuiNotifyFail01(void) {
    Ft_Gpu_CoCmd_Dlstart(&Gpu);
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(255, 4, 4));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(8, 8, 128));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, 80));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, DISP_HEIGHT));
    Ft_Gpu_Hal_WrCmd32(&Gpu,COLOR_RGB(255, 255, 255));
    Ft_Gpu_CoCmd_Text(&Gpu, CONFIG_POS_H_TITLE, CONFIG_POS_V_TITLE, CONFIG_FONT_SIZE, OPT_CENTER, CONFIG_TEXT_TITLE);
    Ft_Gpu_CoCmd_Text(&Gpu, CONFIG_POS_H_INFO,  CONFIG_POS_V_INFO,  CONFIG_FONT_SIZE, OPT_CENTER, "Loading error: 01, wrong format");
    Ft_Gpu_Hal_WrCmd32(&Gpu, DISPLAY());
    Ft_Gpu_CoCmd_Swap(&Gpu);
    Ft_Gpu_Hal_WaitCmdfifo_empty(&Gpu);
}

void appGuiNotifyFail02(void) {
    Ft_Gpu_CoCmd_Dlstart(&Gpu);
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(255, 4, 4));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR_COLOR_RGB(8, 8, 128));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, 80));
    Ft_Gpu_Hal_WrCmd32(&Gpu,CLEAR(1,1,1));
    Ft_Gpu_Hal_WrCmd32(&Gpu,SCISSOR_SIZE(DISP_WIDTH, DISP_HEIGHT));
    Ft_Gpu_Hal_WrCmd32(&Gpu,COLOR_RGB(255, 255, 255));
    Ft_Gpu_CoCmd_Text(&Gpu, CONFIG_POS_H_TITLE, CONFIG_POS_V_TITLE, CONFIG_FONT_SIZE, OPT_CENTER, CONFIG_TEXT_TITLE);
    Ft_Gpu_CoCmd_Text(&Gpu, CONFIG_POS_H_INFO,  CONFIG_POS_V_INFO,  CONFIG_FONT_SIZE, OPT_CENTER, "Loading error: 02, flash erase");
    Ft_Gpu_Hal_WrCmd32(&Gpu, DISPLAY());
    Ft_Gpu_CoCmd_Swap(&Gpu);
    Ft_Gpu_Hal_WaitCmdfifo_empty(&Gpu);
}
