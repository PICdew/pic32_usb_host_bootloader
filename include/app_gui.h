/* 
 * File:   gui.h
 * Author: nenad
 *
 * Created on April 3, 2014, 11:46 PM
 */

#ifndef GUI_H
#define	GUI_H

#ifdef	__cplusplus
extern "C" {
#endif

void initGuiModule(void);
void appGuiInit(void);
void appGuiNotifyErase(void);
void appGuiNotifyLoading(void);
void appGuiNotifyDone(uint32_t);
void appGuiNotifyFail00(void);
void appGuiNotifyFail01(void);
void appGuiNotifyFail02(uint32_t);

#ifdef	__cplusplus
}
#endif

#endif	/* GUI_H */

