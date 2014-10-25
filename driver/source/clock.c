/*
 * File:    clock.c
 * Author:  nenad
 * Details: Clock management
 *
 * Created on February 17, 2014, 1:13 AM
 */

/*=========================================================  INCLUDE FILES  ==*/

#include <peripheral/system.h>
#include <xc.h>

#include "HardwareProfile.h"
#include "driver/clock.h"

/*=========================================================  LOCAL MACRO's  ==*/

#define REFOCON_ROSEL_SYSCLK            (0x0u << 0)
#define REFOCON_ROSEL_PBCLK             (0x1u << 0)
#define REFOCON_ROSEL_FRC               (0x3u << 0)
#define REFOCON_ACTIVE                  (0x1u << 8)
#define REFOCON_OE                      (0x1u << 12)
#define REFOCON_ON                      (0x1u << 15)

#define REFOCON_RODIV_Pos               16
#define REFOCON_RODIV_Msk               (0x7fu << REFOCON_RODIV_Pos)

#define CONFIG_SYSTEM_CLOCK             48000000ul

/*======================================================  LOCAL DATA TYPES  ==*/

struct clocks {
    uint32_t            system;
    uint32_t            peripheralBus;
};

/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/

static struct clocks GlobalClocks;

/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/

void initClockDriver(
    void) {

    GlobalClocks.system        = CONFIG_SYSTEM_CLOCK;
    GlobalClocks.peripheralBus = SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
}

uint32_t clockGetSystemClock(
    void) {

    return (GlobalClocks.system);
}

uint32_t clockGetPeripheralClock (
    void) {

    return (GlobalClocks.peripheralBus);
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of bsp.c
 ******************************************************************************/
