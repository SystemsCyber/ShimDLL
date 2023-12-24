/******************************************************************************************
 * Program Name: simpleRP1210.c
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

/* To compile, use this command: 
    cl.exe simpleRP1210.c
*/
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#include "simpleRP1210.h"

#define PGN4VIN					65260 // The PGN for vehicle identification number (VIN)
#define PGN4VEHICLEDISTANCE     65248 // The PGN for vehicle distance (Odometer)


/* Declare RP1210 functions to be loaded from the external DLL*/
READVERSION RP1210_ReadVersion = NULL;
GETERRORMESSAGE RP1210_GetErrorMsg = NULL;
CLIENTCONNECT RP1210_ClientConnect = NULL;
CLIENTDISCONNECT RP1210_ClientDisconnect = NULL;
SENDCOMMAND RP1210_SendCommand = NULL;
READMESSAGE RP1210_ReadMessage = NULL;
SENDMESSAGE RP1210_SendMessage = NULL;

void main(int argc, // number of strings in the argument array with the program name as 1
          char *argv[] // pointer to the array of command line argument strings
         ){
    
    printf_s("Starting the simpleRP1210 program.\n");
    
    /* Check to see if there are the right amount of arguments. This program takes the first argument to 
    be the dll filename for the RP1210 device and the second argument is the Device ID, which is usually a number.*/
    if (argc < 3){ 
        printf_s("%s requires 2 command line arguments, which are the RP1210 dll file and the device ID.\n", argv[0]);
        printf_s("Example for a DPA5 Dual CAN:\n\t%s DGDPA5MA.dll 1\n\n", argv[0]);
        printf_s("Example for a DPA5 Pro:\n\t%s DGDPA5MA.dll 2\n\n", argv[0]);
        printf_s("Example for a DPAXL:\n\t%s DGDPAXL.dll 1\n\n", argv[0]);
        printf_s("Example for a Cummins Inline 7:\n\t%s CIL7R32.dll 1\n\n", argv[0]);
        printf_s("Example for the Shim DLL:\n\t%s ShimDLL.dll 1\n\n", argv[0]);
        return;
    }

    printf_s("Command-line Arguments:\n");
    for (int i = 0; i < argc; i++){
        printf_s( "argv[%d]: %s\n", i, argv[i]);
    }

    rp1210dll_name = argv[1]; //"CIL7R32.dll"
    printf_s("The name for the RP1210 driver to be used is %s\n", rp1210dll_name);
   
    sDeviceID = atoi(argv[2]);
    printf_s("The RP1210 Device ID selected is %hd\n",sDeviceID);
   
    //check to see if the name has .dll
    char *arg_substring = strstr(rp1210dll_name, ".dll");
    printf_s("Argument substring is %s\n",arg_substring);
    if (strcmp(arg_substring, ".dll") == 0){
        printf_s("Great! The first command line argument has .dll in it. There's a chance it is an RP1210 filename.\n");
    }
    else {
        printf_s("The command line argument did not have .dll in it (case sensitive).\n"\
        "The program requires a command line argument that indicates the .dll file for the RP1210 driver.\n");
        return;
    }
      
    printf_s("Loading external RP1210 library...");
    dll_module = LoadLibrary(rp1210dll_name);
    if (dll_module == NULL)
    {
        printf_s("Couldn't load %s\nCheck to be sure the RP1210 device drivers are "\
        "install on your computer and the right dll file name was specified. Check the "\
        "RP1210432.INI file in the c:\\Windows directory for installed devices.",rp1210dll_name);
        return;
    }
    else {
        printf_s("done.\n");
    }

    printf_s("Loading RP1210 functions...");
    RP1210_ReadVersion = GetProcAddress(dll_module, "RP1210_ReadVersion");
    RP1210_GetErrorMsg = GetProcAddress(dll_module, "RP1210_GetErrorMsg");
    RP1210_ClientConnect = GetProcAddress(dll_module, "RP1210_ClientConnect");
    RP1210_ClientDisconnect = GetProcAddress(dll_module, "RP1210_ClientDisconnect");
    RP1210_SendCommand = GetProcAddress(dll_module, "RP1210_SendCommand");
    RP1210_ReadMessage = GetProcAddress(dll_module, "RP1210_ReadMessage");
    RP1210_SendMessage = GetProcAddress(dll_module, "RP1210_SendMessage");
    //Add other functions here
    printf_s("done.\n");

    if (RP1210_ClientConnect == NULL){
        printf("There's a problem. RP1210_ClientConnect is Null.\n");
        FreeLibrary(dll_module);
        return;
    }

    /* Test out the read version value. */
    printf_s("Using RP1210_ReadVersion...");
    /* This function is exposed on the DG drivers, but not the Cummins Inline 7 */
    /* No hardware is needed to connect to the computer, since it is only looking 
    at the DLL file. */
    char *fpchDLLMajorVersion="";
    char *fpchDLLMinorVersion="";
    char *fpchAPIMajorVersion="";
    char *fpchAPIMinorVersion="";
    RP1210_ReadVersion( fpchDLLMajorVersion,
                        fpchDLLMinorVersion,
                        fpchAPIMajorVersion,
                        fpchAPIMinorVersion);
    printf_s("done.\n");
    /* use %c instead of $s to get a single character from the pointer. */
    /* The [0] at the end returnd a single value for the %c printing.  */
    printf_s("DLL Major Version: %c\n", fpchDLLMajorVersion[0]);
    printf_s("DLL Minor Version: %c\n", fpchDLLMinorVersion[0]);
    printf_s("API Major Version: %c\n", fpchAPIMajorVersion[0]);
    printf_s("API Minor Version: %c\n", fpchAPIMinorVersion[0]);
    

    /* Let's connect to a J1939 client so we can explore the rest of the RP1210 API*/
    printf_s("Using RP1210_ClientConnect...");
    //HWND, short, char *, long, long, short
    HWND hwnd = NULL;
    int j1939Client = RP1210_ClientConnect( // the j1939Client is an integer that identifies the communication session
        hwnd, //the first HWND parameter is not used anymore. It was for old Windows 3.1 implementations
		sDeviceID,
		"J1939:Baud=Auto,Channel=1", 
		0, 
		0,
		FALSE); 
    printf_s("done.\n");
    printf_s("The J1939 Client Identifier is %d\n",j1939Client);
    if (j1939Client > 127){
        printf_s("There is and issue in connecting to the client. Please be sure to have a the correct device "\
        "plugged in and connected to an operating J1939 network. The device and DLL and device ID must match. "\
        "The error code of %d can be looked up in teh RP1210 documentation.",j1939Client);
        RP1210_ClientDisconnect(j1939Client);
        FreeLibrary(dll_module);
        return;
    }

    /* Setup command to have all messages pass.*/
    printf_s("Using RP1210_SendCommand for set filters to pass...");
    char filterBuffer[200]="";
    ret_val = RP1210_SendCommand(
        RP1210_SET_FILTERS_TO_PASS, 
		j1939Client,
		filterBuffer,
		0);
	printf_s("done.\n");
    printf_s("The return code for setting filters to pass is %d\n",ret_val);
	if (ret_val > 127 ){
		RP1210_ClientDisconnect(j1939Client);
        FreeLibrary(dll_module);
        return;	
	}

    /*Listen for message traffic. We need to be connected to a live network for this.*/
    time_t start_time = time(NULL);
    while (!_kbhit()){
        /*Clear the current buffer to make all values null*/
        memset(ucTxRxBuffer,0,sizeof(ucTxRxBuffer));
        ret_val = RP1210_ReadMessage(
            j1939Client, //client handle
            &ucTxRxBuffer[0], //pointer to the buffer where read data goes
            TXRXSIZE,      //maximum size of read message                
            TRUE);    //Blocking setting

        if (ret_val == 0){
            //no more messages in buffer. Let's sleep while a message arrives
            //Sleep(.0001); 
        }
        else if (ret_val > 0) { //ret_val is the size of message that was read
            ulReadCount++;
            printf_s("%06d: %2d ",ulReadCount,ret_val);
            /* Print out the buffer and the ASCII of the data*/
            for (int i = 0; i < ret_val; i++){
                printf_s("%02X ", ucTxRxBuffer[i] );
            }
            printf_s("  ");
            for (int i = DATASTART_J1939; i < ret_val; i++){
                printf_s("%c", ucTxRxBuffer[i] );
            }
            printf("\n");

            /* The ucTxRxBuffer for a J1939 client has this structure:
                +------------+--------+-----+--------------+-------------+-----------+--------------+
                | Timestamp  | Echo** | PGN | How/Priority | Source Addr | Dest Addr | Message Data |
                +------------+--------+-----+--------------+-------------+-----------+--------------+
                |     4      | 0 or 1 |  3  |       1      |     1       |      1    |  0 to 1785   |
                +------------+--------+-----+--------------+-------------+-----------+--------------+
                **If the echo feature is on for sent messages, then this byte will be present. 
                  Otherwise, it won't be used. In other words, if ECHO MODE is off, then this field 
                  will not be in the message buffer.
            */

            /*Determine PGN when ECHO MODE is off*/
            /* this converss the 3 bytes for the PGN into a 24-bit number. Actual PGNs are 18 bits.*/
            unsigned long pgn =  ucTxRxBuffer[4] + (ucTxRxBuffer[5] << 8) + (ucTxRxBuffer[6] << 16);
            if (pgn == PGN4VIN){ // Look for the VIN to break
                printf_s("Found PGN %d\n",pgn);
                break;
            }
            else if (pgn == PGN4VEHICLEDISTANCE){
                printf_s("Found PGN %d\n",pgn);
                //break;
            }
        }
        else //negative number indicates an error
        {
            printf("RP1210 Read error code: %d\n", ret_val * -1);
        }
        
        //While there are messages coming in, let's send out request message for VIN.
        /* We'll do this based on a timer. */
        time_t current_time = time(NULL);
        if (current_time - start_time > VIN_REQUEST_INTERVAL) {
            // Reset the start time
            start_time = current_time;

            printf_s("Using RP1210_SendMEssage to request VIN...");
            /* Send out Request message for VIN                                 */
            /* The CAN message for this request is 18EAFFF9 [3] EC FE 00        */
            /* which is a priority 6 message with a PGN of 59904 (0xEA00),      */
            /* the destination address is the GLOBAL address of 255 (0xFF),     */
            /* the source address is for an offboard tool 249 (0xF9),           */
            /* and the 3 data bytes that get sent are the PGN for the VIN,      */
            /* which is 65260 (0x00FEEC). In reverse byte order it is EC FE 00  */

            /* Based on the needed format for J1939 messages, we have to populate the buffer for this message.*/
            /* See the RP1210 API: Formatting a J1939 Message for RP1210_SendMessage  for more detail.*/
            /*
                +-----+--------------+-------------+-----------+--------------+
                | PGN | How/Priority | Source Addr | Dest Addr | Message Data |
                +-----+--------------+-------------+-----------+--------------+
                |  3  |      1       |     1       |     1     |  0 - 1785    |
                +-----+--------------+-------------+-----------+--------------+
            */
            memset(ucTxRxBuffer,0,sizeof(ucTxRxBuffer));
            ucTxRxBuffer[0] = 0x00; //PGN LSB
            ucTxRxBuffer[1] = 0xEA; //PGN
            ucTxRxBuffer[2] = 0x00; //PGN MSB
            ucTxRxBuffer[3] = 0x06; //Priority
            ucTxRxBuffer[4] = 0xF9; //Source Address
            ucTxRxBuffer[5] = 0xFF; //Destination Address
            ucTxRxBuffer[6] = 0xEC; //Requested PGN LSB
            ucTxRxBuffer[7] = 0xFE; //Requested PGN 
            ucTxRxBuffer[8] = 0x00; //Requested PGN MSB
            
            ret_val = RP1210_SendMessage(
                j1939Client,  //client handle
                ucTxRxBuffer, //message we want to send
                3 + 1 + 1 + 1 + 3,   //length = sum(3 for PGN, 1 for priority, 1 for Source, 1 for dest, 3 for data)
                FALSE,        //must be false
                FALSE);       //must be false
            printf_s("done.\n");
            if (ret_val > 0) {
                printf_s("Return Value for Send Message is: %d\n", ret_val);
            }
            else { 
                ulSendCount++; 
            }
        }
    }
    
    printf("\nProgram ending.\nNumber of SendMessages %d\nNumber ReadMessages %d\n", ulSendCount, ulReadCount);
    
    /*Clean things up before exiting.*/
    RP1210_ClientDisconnect(j1939Client);
    FreeLibrary(dll_module);
    return;
}
