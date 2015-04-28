/*
 * This file is part of tft
 *
 * Template version: 1.1.16 (24.12.2013)
 *
 * Copyright (C) 2011, 2012 - Nenad Radulovic
 *
 * tft is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * tft is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with eSolid.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://blueskynet.dyndns-server.com
 * e-mail  :    blueskyniss@gmail.com
 *//***********************************************************************//**
 * @file
 * @author      Administrator
 * @brief       Short desciption of file
 * @addtogroup  module_impl
 *********************************************************************//** @{ */
                                                                                                  
/*=========================================================  INCLUDE FILES  ==*/

#include <xc.h>
#include "gui.h"
#include "ILI9341.h"

/*=========================================================  LOCAL MACRO's  ==*/


/*======================================================  LOCAL DATA TYPES  ==*/
/*=============================================  LOCAL FUNCTION PROTOTYPES  ==*/
/*=======================================================  LOCAL VARIABLES  ==*/
/*======================================================  GLOBAL VARIABLES  ==*/
/*============================================  LOCAL FUNCTION DEFINITIONS  ==*/
/*===================================  GLOBAL PRIVATE FUNCTION DEFINITIONS  ==*/
/*====================================  GLOBAL PUBLIC FUNCTION DEFINITIONS  ==*/
void gui_init(void) {
    TFT_LCD_DATA_TRIS       &= ~0xffu;
    TFT_LCD_CS_TRIS         &= ~(0x1u << TFT_LCD_CS_PIN);
    TFT_LCD_DC_TRIS         &= ~(0x1u << TFT_LCD_DC_PIN);
    TFT_LCD_WR_TRIS         &= ~(0x1u << TFT_LCD_WR_PIN);
    TFT_LCD_RD_TRIS         &= ~(0x1u << TFT_LCD_RD_PIN);
    TFT_LCD_RESET_TRIS      &= ~(0x1u << TFT_LCD_RESET_PIN);
    TFT_LCD_BACKLIGHT_TRIS  &= ~(0x1u << TFT_LCD_RESET_PIN);
    TFT_LCD_BACKLIGHT_PORT  |=  (0x1u << TFT_LCD_BACKLIGHT_PIN);
    lcdInit();
}

/*================================*//** @cond *//*==  CONFIGURATION ERRORS  ==*/
/** @endcond *//** @} *//******************************************************
 * END of gui.c
 ******************************************************************************/
