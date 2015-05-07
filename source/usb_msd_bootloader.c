
#include <stdbool.h>
#include <stdint.h>
#include <plib.h>

#include "MDD File System/FSIO.h"
#include "NVMem.h"

#include "BootLoader.h"
#include "GenericTypeDefs.h"

#include "HardwareProfile.h"
#include "mcu_config.h"
#include "usb.h"
#include "usb_host_msd.h"

#include "driver/clock.h"
#include "driver/intr.h"
#include "driver/gpio.h"

#include "app_gui.h"
#include "app_buzzer.h"
#include "TimeDelay.h"

/******************************************************************************
Macros used in this file
*******************************************************************************/

#define CONFIG_IMAGE_FILE_NAME          "glucofw.hex"
#define CONFIG_TIMER_WAIT_USB           800000
#define CONFIG_TIMER_NOTIFY_FAIL        5000

#define DEV_CONFIG_REG_BASE_ADDRESS     0x9FC02FF0
#define DEV_CONFIG_REG_END_ADDRESS      0x9FC02FFF

#define DATA_RECORD                     0
#define END_OF_FILE_RECORD              1
#define EXT_SEG_ADRS_RECORD             2
#define EXT_LIN_ADRS_RECORD             4

#define REC_FLASHED                     0
#define REC_NOT_FOUND                   1
#define REC_FOUND_BUT_NOT_FLASHED       2

#define FLASH_PAGE_SIZE                 0x1000

typedef struct {
    UINT8 *start;
    UINT8 len;
    UINT8 status;
} T_REC;

typedef struct {
	UINT8 RecDataLen;
	DWORD_VAL Address;
	UINT8 RecType;
	UINT8* Data;
	UINT8 CheckSum;
	DWORD_VAL ExtSegAddress;
	DWORD_VAL ExtLinAddress;
} T_HEX_RECORD;

enum mainFsmState {
    FSM_INIT,
    FSM_WAIT_FOR_USB,
    FSM_INIT_FS,
    FSM_OPEN_FILE,
    FSM_NOTIFY_ERASE,
    FSM_NOTIFY_LOAD,
    FSM_NOTIFY_DONE,
    FSM_LOAD_FILE,
    FSM_CLOSE_FILE,
    FSM_NOTIFY_FAIL,
    FSM_BOOT_APP
};

static uint32_t         FileSize = 0;

static void bootloader(void);
static void jumpToApp(void);
static void convertAsciiToHex(UINT8* asciiRec, UINT8* hexRec);
static void eraseFlash(void);
static void writeHexRecord2Flash(UINT8 * HexRecord);
static bool isValidAppPresent(void);
static uint32_t getFlashNonEmptyAddress(void);

static void bootloader(void) {
    static enum mainFsmState state = FSM_INIT;
    static FSFILE *         imageFile;
    static T_REC            record;
    static uint32_t         readBytes;
    static uint32_t         pointer;
    static uint32_t         retry;
    static uint8_t          asciiBuffer[1024];
    
    
    switch (state) {
        case FSM_INIT: {
            retry = CONFIG_TIMER_WAIT_USB;
            state = FSM_WAIT_FOR_USB;
            appGuiInit();
            appGuiNotifySearching();
            break;
        }
        case FSM_WAIT_FOR_USB: {
            retry--;

            if (retry == 0u) {
                state = FSM_BOOT_APP;
            }

            if (USBHostMSDSCSIMediaDetect()) {
                state = FSM_INIT_FS;
            }
            break;
        }
        case FSM_INIT_FS: {

            if (FSInit()) {
                state = FSM_OPEN_FILE;
            }
            break;
        }
        case FSM_OPEN_FILE: {
            imageFile = FSfopen(CONFIG_IMAGE_FILE_NAME, "r");

            if (imageFile != NULL) {
                
                /*--  Initialize the state-machine to read the records  ------*/
                record.status = REC_NOT_FOUND;
                state = FSM_NOTIFY_ERASE;
            } else {
                state = FSM_BOOT_APP;
            }
            break;
        }
        case FSM_NOTIFY_ERASE: {
            uint32_t    address;

            buzzerTone(20);
            
            appGuiNotifyErase();
            eraseFlash();
            address = getFlashNonEmptyAddress();

            if (address == 0) {
                state = FSM_NOTIFY_LOAD;
            } else {
                appGuiNotifyFail02(address);

                while(1);
            }

            break;
        }
        case FSM_NOTIFY_LOAD: {
            appGuiNotifyLoading();
            state = FSM_LOAD_FILE;

            break;
        }
        case FSM_LOAD_FILE: {
            /*--  For a faster read, read 512 bytes at a time and buffer it.  */
            readBytes = FSfread((void *)&asciiBuffer[pointer], 1, 512, imageFile);
            FileSize += readBytes;

            if (readBytes == 0) {
                state = FSM_CLOSE_FILE;
            } else {
                uint32_t byteCount;
                uint8_t  hexRec[100];

                for (byteCount = 0; byteCount < (readBytes + pointer); byteCount ++) {
                    // This state machine seperates-out the valid hex records from the read 512 bytes.
                    switch (record.status) {
                        case REC_FLASHED:
                        case REC_NOT_FOUND: {
                            if (asciiBuffer[byteCount] == ':') {
                                // We have a record found in the 512 bytes of data in the buffer.
                                record.start = &asciiBuffer[byteCount];
                                record.len = 0;
                                record.status = REC_FOUND_BUT_NOT_FLASHED;
                            }
                            break;
                        }
                        case REC_FOUND_BUT_NOT_FLASHED: {
                            if ((asciiBuffer[byteCount] == 0x0A) || (asciiBuffer[byteCount] == 0xFF)) {
                                // We have got a complete record. (0x0A is new line feed and 0xFF is End of file)
                                // Start the hex conversion from element
                                // 1. This will discard the ':' which is
                                // the start of the hex record.
                                convertAsciiToHex(&record.start[1], hexRec);
                                writeHexRecord2Flash(hexRec);
                                record.status = REC_FLASHED;
                            }
                            break;
                        }
                    }
                    // Move to next byte in the buffer.
                    record.len ++;
                }

                if (record.status == REC_FOUND_BUT_NOT_FLASHED) {
                    // We still have a half read record in the buffer. The next half part of the record is read
                    // when we read 512 bytes of data from the next file read.
                    memcpy(asciiBuffer, record.start, record.len);
                    pointer = record.len;
                    record.status = REC_NOT_FOUND;
                } else {
                    pointer = 0;
                }
            }
            break;
        }
        case FSM_CLOSE_FILE: {
            if (FSfclose(imageFile) != 0) {
                appGuiNotifyFail00();
                state = FSM_NOTIFY_FAIL;
            } else {
                state = FSM_NOTIFY_DONE;
            }

            break;
        }
        case FSM_NOTIFY_DONE: {
            buzzerTone(20);
            appGuiNotifyDone(FileSize);
            DelayMs(5000);
            state = FSM_BOOT_APP;

            break;
        }
        case FSM_BOOT_APP: {
            
            if (isValidAppPresent()) {
                jumpToApp();
            } else {
                buzzerTone(20);
                appGuiNotifyFail00();
                state = FSM_NOTIFY_FAIL;
            }
            break;
        }
        case FSM_NOTIFY_FAIL: {
            
            break;
        }
    }
}

/********************************************************************
* Function: 	JumpToApp()
*
* Precondition:
*
* Input: 		None.
*
* Output:
*
* Side Effects:	No return from here.
*
* Overview: 	Jumps to application.
*
*
* Note:		 	None.
********************************************************************/
static void jumpToApp(void) {
    void (*fptr)(void);
	fptr = (void (*)(void))USER_APP_RESET_ADDRESS;
	fptr();
}

/********************************************************************
* Function: 	ConvertAsciiToHex()
*
* Precondition:
*
* Input: 		Ascii buffer and hex buffer.
*
* Output:
*
* Side Effects:	No return from here.
*
* Overview: 	Converts ASCII to Hex.
*
*
* Note:		 	None.
********************************************************************/
static void convertAsciiToHex(UINT8 * asciiRec, UINT8 * hexRec) {
	UINT8 i = 0;
	UINT8 k = 0;
	UINT8 hex;


	while((asciiRec[i] >= 0x30) && (asciiRec[i] <= 0x66)) {
		// Check if the ascci values are in alpha numeric range.

		if(asciiRec[i] < 0x3A) {
			// Numerical reperesentation in ASCII found.
			hex = asciiRec[i] & 0x0F;
		} else {
			// Alphabetical value.
			hex = 0x09 + (asciiRec[i] & 0x0F);
		}

		// Following logic converts 2 bytes of ASCII to 1 byte of hex.
		k = i%2;

		if (k) {
			hexRec[i/2] |= hex;
		} else {
			hexRec[i/2] = (hex << 4) & 0xF0;
		}
		i++;
	}
}

/********************************************************************
* Function: 	EraseFlash()
*
* Precondition:
*
* Input: 		None.
*
* Output:
*
* Side Effects:	No return from here.
*
* Overview: 	Erases Flash (Block Erase).
*
*
* Note:		 	None.
********************************************************************/
static void eraseFlash(void) {
    uint32_t result;
    uint32_t i;

    for( i = APP_FLASH_BASE_ADDRESS; i < APP_FLASH_END_ADDRESS; i+=FLASH_PAGE_SIZE )
    {
	    result = NVMemErasePage((void *)i);
        // Assert on NV error. This must be caught during debug phase.

        if(result != 0)
        {
           // We have a problem. This must be caught during the debug phase.
            appGuiNotifyFail02(i);
            
            while(1);
        }
    }
}



/********************************************************************
* Function: 	WriteHexRecord2Flash()
*
* Precondition:
*
* Input: 		None.
*
* Output:
*
* Side Effects:	No return from here.
*
* Overview: 	Writes Hex Records to Flash.
*
*
* Note:		 	None.
********************************************************************/
static void writeHexRecord2Flash(UINT8 * HexRecord) {
	static T_HEX_RECORD HexRecordSt;
	UINT8 Checksum = 0;
	UINT8 i;
	UINT WrData;
	void* ProgAddress;
	UINT result;

	HexRecordSt.RecDataLen = HexRecord[0];
	HexRecordSt.RecType = HexRecord[3];
	HexRecordSt.Data = &HexRecord[4];

	// Hex Record checksum check.
	for(i = 0; i < HexRecordSt.RecDataLen + 5; i++)
	{
		Checksum += HexRecord[i];
	}

    if(Checksum != 0)
    {
	    //Error. Hex record Checksum mismatch.
	    //Indicate Error by switching ON all LEDs.
	    appGuiNotifyFail01();
	    // Do not proceed further.
	    while(1);
	}
	else
	{
		// Hex record checksum OK.
		switch(HexRecordSt.RecType)
		{
			case DATA_RECORD:  //Record Type 00, data record.
				HexRecordSt.Address.byte.MB = 0;
					HexRecordSt.Address.byte.UB = 0;
					HexRecordSt.Address.byte.HB = HexRecord[1];
					HexRecordSt.Address.byte.LB = HexRecord[2];

					// Derive the address.
					HexRecordSt.Address.Val = HexRecordSt.Address.Val + HexRecordSt.ExtLinAddress.Val + HexRecordSt.ExtSegAddress.Val;

					while(HexRecordSt.RecDataLen) // Loop till all bytes are done.
					{

						// Convert the Physical address to Virtual address.
						ProgAddress = (void *)PA_TO_KVA0(HexRecordSt.Address.Val);

						// Make sure we are not writing boot area and device configuration bits.
						if(((ProgAddress >= (void *)APP_FLASH_BASE_ADDRESS) && (ProgAddress < (void *)APP_FLASH_END_ADDRESS))
						   && ((ProgAddress < (void*)DEV_CONFIG_REG_BASE_ADDRESS) || (ProgAddress > (void*)DEV_CONFIG_REG_END_ADDRESS)))
						{
							if(HexRecordSt.RecDataLen < 4)
							{

								// Sometimes record data length will not be in multiples of 4. Appending 0xFF will make sure that..
								// we don't write junk data in such cases.
								WrData = 0xFFFFFFFF;
								memcpy(&WrData, HexRecordSt.Data, HexRecordSt.RecDataLen);
							}
							else
							{
								memcpy(&WrData, HexRecordSt.Data, 4);
							}
							// Write the data into flash.
							result = NVMemWriteWord(ProgAddress, WrData);
							// Assert on error. This must be caught during debug phase.
							if(result != 0)
							{
    							while(1);
    						}
						}

						// Increment the address.
						HexRecordSt.Address.Val += 4;
						// Increment the data pointer.
						HexRecordSt.Data += 4;
						// Decrement data len.
						if(HexRecordSt.RecDataLen > 3)
						{
							HexRecordSt.RecDataLen -= 4;
						}
						else
						{
							HexRecordSt.RecDataLen = 0;
						}
					}
					break;

			case EXT_SEG_ADRS_RECORD:  // Record Type 02, defines 4th to 19th bits of the data address.
			    HexRecordSt.ExtSegAddress.byte.MB = 0;
				HexRecordSt.ExtSegAddress.byte.UB = HexRecordSt.Data[0];
				HexRecordSt.ExtSegAddress.byte.HB = HexRecordSt.Data[1];
				HexRecordSt.ExtSegAddress.byte.LB = 0;
				// Reset linear address.
				HexRecordSt.ExtLinAddress.Val = 0;
				break;

			case EXT_LIN_ADRS_RECORD:   // Record Type 04, defines 16th to 31st bits of the data address.
				HexRecordSt.ExtLinAddress.byte.MB = HexRecordSt.Data[0];
				HexRecordSt.ExtLinAddress.byte.UB = HexRecordSt.Data[1];
				HexRecordSt.ExtLinAddress.byte.HB = 0;
				HexRecordSt.ExtLinAddress.byte.LB = 0;
				// Reset segment address.
				HexRecordSt.ExtSegAddress.Val = 0;
				break;

			case END_OF_FILE_RECORD:  //Record Type 01, defines the end of file record.
				HexRecordSt.ExtSegAddress.Val = 0;
				HexRecordSt.ExtLinAddress.Val = 0;
				// Disable any interrupts here before jumping to the application.
				//IEC1bits.USBIE = 0;
				USBDisableInterrupts();
                buzzerTone(20);
                appGuiNotifyDone(FileSize);
                DelayMs(5000);
                jumpToApp();

				break;

			default:
				HexRecordSt.ExtSegAddress.Val = 0;
				HexRecordSt.ExtLinAddress.Val = 0;
				break;
		}
	}
}

static bool isValidAppPresent(void) {
	volatile uint32_t * app;

	app = (uint32_t *)USER_APP_RESET_ADDRESS;

	if (*app == 0xFFFFFFFF) {

        return (false);
	} else {

        return (true);
	}
}

static uint32_t getFlashNonEmptyAddress(void) {
    volatile uint32_t * address;
    uint32_t            retval;

    retval = 0;

    for (address = (uint32_t *)APP_FLASH_BASE_ADDRESS; address < (uint32_t *)APP_FLASH_END_ADDRESS; address++) {

        if (*address != 0xffffffff) {
            retval = (uint32_t)address;

            break;
        }
    }

    return (retval);
}

/****************************************************************************
  Function:
    BOOL USB_ApplicationEventHandler( BYTE address, USB_EVENT event,
                void *data, DWORD size )

  Summary:
    This is the application event handler.  It is called when the stack has
    an event that needs to be handled by the application layer rather than
    by the client driver.

  Description:
    This is the application event handler.  It is called when the stack has
    an event that needs to be handled by the application layer rather than
    by the client driver.  If the application is able to handle the event, it
    returns TRUE.  Otherwise, it returns FALSE.

  Precondition:
    None

  Parameters:
    BYTE address    - Address of device where event occurred
    USB_EVENT event - Identifies the event that occured
    void *data      - Pointer to event-specific data
    DWORD size      - Size of the event-specific data

  Return Values:
    TRUE    - The event was handled
    FALSE   - The event was not handled

  Remarks:
    The application may also implement an event handling routine if it
    requires knowledge of events.  To do so, it must implement a routine that
    matches this function signature and define the USB_HOST_APP_EVENT_HANDLER
    macro as the name of that function.
  ***************************************************************************/

BOOL USB_ApplicationEventHandler( BYTE address, USB_EVENT event, void *data, DWORD size )
{
    switch( event )
    {
        case EVENT_VBUS_REQUEST_POWER:
            // The data pointer points to a byte that represents the amount of power
            // requested in mA, divided by two.  If the device wants too much power,
            // we reject it.
            return TRUE;

        case EVENT_VBUS_RELEASE_POWER:
            // Turn off Vbus power.
            // The PIC24F with the Explorer 16 cannot turn off Vbus through software.

            //This means that the device was removed
            //deviceAttached = FALSE;
            return TRUE;
            break;

        case EVENT_HUB_ATTACH:
            return TRUE;
            break;

        case EVENT_UNSUPPORTED_DEVICE:
            return TRUE;
            break;

        case EVENT_CANNOT_ENUMERATE:
            //UART2PrintString( "\r\n***** USB Error - cannot enumerate device *****\r\n" );
            return TRUE;
            break;

        case EVENT_CLIENT_INIT_ERROR:
            //UART2PrintString( "\r\n***** USB Error - client driver initialization error *****\r\n" );
            return TRUE;
            break;

        case EVENT_OUT_OF_MEMORY:
            //UART2PrintString( "\r\n***** USB Error - out of heap memory *****\r\n" );
            return TRUE;
            break;

        case EVENT_UNSPECIFIED_ERROR:   // This should never be generated.
            //UART2PrintString( "\r\n***** USB Error - unspecified *****\r\n" );
            return TRUE;
            break;

        default:
            break;
    }

    return FALSE;
}

int main(void) {

    /* Turn off VOC sensor power supply */
    TRISD &= ~(0x1u << 10);
    LATD  |=  (0x1u << 10);
    TRISD &= ~(0x1u << 5 );
    LATD  &= ~(0x1u << 5 );
    initClockDriver();
    initIntrDriver();
    initGuiModule();
    initBuzzerModule();
    USBInitialize(0);

    while(1) {
        USBTasks();
        bootloader();
    }
    
    return (0);
}



