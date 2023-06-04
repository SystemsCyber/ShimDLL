/******************************************************************************************
 * Program Name: simpleRP1210.h (header file for the C program)
 * 
 * Description: A proof of concept written to be compiled with MSVC on Windows
 * to interface with basic functionality of American Trucking Association,
 * Technology Maintenance Council (TMC) Recommended Practice number 1210.
 * This Windows API for truck diagnostics tools gives us the ability to write programs
 * that can use the device. 
 * The API can be purchases through the ATA. At one point in time (2023) the  RPs could
 * be purchased through the web: https://tmc.trucking.org/TMC-Recommended-Practices
 * 
 * Author: Jeremy Daily
 * 
 * Copyright © 2023 Jeremy Daily
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the “Software”), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <windows.h>

#define VIN_REQUEST_INTERVAL 	2 // Number of seconds to pass before trying again
#define REQEST_PGN 				0xEA00 // The J1939 parameter group number to make requests
#define TOOL_SOURCE_ADDRESS 	249 // The Source address for an off-board diagnostics tool
#define GLOBAL_ADDRESS			255 // The J1939 global source address
#define PGN4VIN					65260 // The PGN for vehicle identification number (VIN)
#define TXRXSIZE 				4200 // A large buffer for RP1210 messages. The largest one is 4096 for ISO TP messages
#define DATASTART_J1939			10 // The start position in the buffer for J1939 data

/* initialize the buffer used to transmit and receive data with the device.*/
unsigned char ucTxRxBuffer[TXRXSIZE];

/* This DeviceID should be passed into the program by argument. The device IDs are in the ini files.*/
short sDeviceID = 1;
unsigned char ucDeviceID[4];

unsigned long ulSendCount, ulReadCount;

int ret_val;

HINSTANCE dll_module; //This declaration came from looking a the compiler warnings when assigning it to the output of LoadLibrary
char *rp1210dll_name;

/***********************/
/* function Types from RP1210 manual*/
/***********************/
typedef int  (WINAPI *CLIENTCONNECT) (HWND, short, char *, long, long, int);
typedef int  (WINAPI *SENDCOMMAND)(short, short, char *, short);
typedef int  (WINAPI *READMESSAGE)(short, char *, short, short);
typedef int  (WINAPI *SENDMESSAGE)(short, char *, short, short, short);
typedef int  (WINAPI *CLIENTDISCONNECT) (short);
typedef int  (WINAPI *GETHARDWARESTATUS)(short, char *, short, short);
typedef int  (WINAPI *GETHARDWARESTATUSEX)(short, char *, short, short);
typedef int  (WINAPI *GETERRORMESSAGE)(short, char *);
typedef void (WINAPI *READVERSION)(char *, char *, char *, char *);
typedef int  (WINAPI *IOCTL)(short, long, void *, void *);
typedef int  (WINAPI *READDETAILEDVERSION)(short, char *, char *, char *);

/*	Constants	*/
/*	RP1210D SendCommand Defines	*/
/*  There are many defines in the RP1210 API. This is incomplete.*/
#define PP1210_RESET_DEVICE					0
#define RP1210_SET_FILTERS_TO_PASS			3
