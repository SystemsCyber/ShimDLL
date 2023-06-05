/************************
 * Use this to compile:
 * 
cl.exe /nologo /LD /EHsc /W4 /D_USRDLL /D_WINDLL shimDLL.c  /link /OUT:shimDLL.dll /DEF:RP1210.def

Use this to test:

simpleRP1210.exe shimDLL.dll 1

*/
#include "shimDLL.h"
//#include "simpleRP1210.h"
#include <stdio.h>
#include <string.h>
//#include <socketapi.h>

HMODULE dll_module;
FILE* hLogFile;
const char* logFileName = "log.txt";
const char* xternal_rp1210dll_name = "DGDPAXL.dll";

char databytes[4];
char messagelog[500];

/* Declare RP1210 functions to be loaded from the external DLL*/
READVERSION Xternal_RP1210_ReadVersion = NULL;
GETERRORMESSAGE Xternal_RP1210_GetErrorMsg = NULL;
CLIENTCONNECT Xternal_RP1210_ClientConnect = NULL;
CLIENTDISCONNECT Xternal_RP1210_ClientDisconnect = NULL;
SENDCOMMAND Xternal_RP1210_SendCommand = NULL;
READMESSAGE Xternal_RP1210_ReadMessage = NULL;
SENDMESSAGE Xternal_RP1210_SendMessage = NULL;


short __declspec(dllexport) WINAPI RP1210_ClientConnect( 
                                        HWND    hwndClient,
										short 	nDeviceID,
                                       	const char *fpchProtocol,
                                       	long	lTxBufferSize,
                                        long    lRxBufferSize,
                                        short   nIsAppPacketizingInMsgs ){
    if (Xternal_RP1210_ClientConnect == NULL){
        dll_module = LoadLibrary(xternal_rp1210dll_name);
        Xternal_RP1210_ReadVersion = GetProcAddress(dll_module, "RP1210_ReadVersion");
        Xternal_RP1210_GetErrorMsg = GetProcAddress(dll_module, "RP1210_GetErrorMsg");
        Xternal_RP1210_ClientConnect = GetProcAddress(dll_module, "RP1210_ClientConnect");
        Xternal_RP1210_ClientDisconnect = GetProcAddress(dll_module, "RP1210_ClientDisconnect");
        Xternal_RP1210_SendCommand = GetProcAddress(dll_module, "RP1210_SendCommand");
        Xternal_RP1210_ReadMessage = GetProcAddress(dll_module, "RP1210_ReadMessage");
        Xternal_RP1210_SendMessage = GetProcAddress(dll_module, "RP1210_SendMessage");
    }
    int status = ERR_DLL_NOT_INITIALIZED;

    status = Xternal_RP1210_ClientConnect(hwndClient, nDeviceID, 
			fpchProtocol, lTxBufferSize, lRxBufferSize, nIsAppPacketizingInMsgs);
    
    sprintf_s(messagelog,sizeof(messagelog),
        "\n\nXX,CC,%02d,%d,\"%s\",%ld,%ld,%d",
		status,nDeviceID,fpchProtocol,lTxBufferSize,lRxBufferSize,nIsAppPacketizingInMsgs );
    fopen_s(&hLogFile, logFileName, "a+");
    fwrite(messagelog, sizeof(char), strlen(messagelog), hLogFile);
    fclose(hLogFile);

    return(status);
}

short __declspec(dllexport) WINAPI RP1210_ClientDisconnect( 
                                        short	nClientID ){
    int status = ERR_DLL_NOT_INITIALIZED;
    if (Xternal_RP1210_ClientDisconnect != NULL){
        status = Xternal_RP1210_ClientDisconnect(nClientID);
    }
    char messagelog[200];
    sprintf_s(messagelog,200,
        "%02d,CD,%02d\n",
		status,nClientID);
    fopen_s(&hLogFile, logFileName, "a+");
    fwrite(messagelog, sizeof(char), strlen(messagelog), hLogFile);
    fclose(hLogFile);

    return(status);
}

short __declspec(dllexport) WINAPI RP1210_SendMessage( 
                                        short	nClientID,
                                        unsigned char *fpchClientMessage,
                                        short	nMessageSize,
                                        short   nNotifyStatusOnTx,
                                        short	nBlockOnSend ){
    int status = ERR_DLL_NOT_INITIALIZED;
    if (Xternal_RP1210_SendMessage != NULL){
        status = Xternal_RP1210_SendMessage(nClientID,
					fpchClientMessage,
					nMessageSize,
					nNotifyStatusOnTx,
					nBlockOnSend);
    }
    // /* Make a print of the data bytes*/
    // char databytes[4100];
    // for (int i = 0; i<(3*nMessageSize); i+=3){
    //     sprintf_s(&databytes[i],3,",%02X", fpchClientMessage[i] );
    // }
    // char messagelog[5000];
    // sprintf_s(messagelog,5000,
    //     "%02d,SM,%02d,%02d,%02d,%d%s\n",
	// 	status,nClientID,nBlockOnSend,nNotifyStatusOnTx,nMessageSize,databytes);
    // fopen_s(&hLogFile, logFileName, "a+");
    // fwrite(messagelog, sizeof(char), strlen(messagelog), hLogFile);
    // fclose(hLogFile);

    return(status);                                           
}

short __declspec(dllexport) WINAPI RP1210_ReadMessage(
                                        short   nClientID,
										unsigned char *fpchAPIMessage,
                                        short	nBufferSize,
                                        short	nBlockOnRead ){
    int status = ERR_DLL_NOT_INITIALIZED;
    if (Xternal_RP1210_ReadMessage != NULL){
        status = Xternal_RP1210_ReadMessage(nClientID,
					fpchAPIMessage,
					nBufferSize,
					nBlockOnRead);
    }
    /* Make a print of the data bytes*/

    sprintf_s(messagelog,sizeof(messagelog),
        "\n%02d,RM,%d,%d,%d",
		nClientID,nBlockOnRead,nBufferSize,status);
    fopen_s(&hLogFile, logFileName, "a+");
    fwrite(messagelog, sizeof(char), strlen(messagelog), hLogFile);
    for (int i = 0; i<(status); i++){
        sprintf_s(databytes,4,",%02X", fpchAPIMessage[i] );
        fwrite(databytes,sizeof(char), strlen(databytes), hLogFile);
    }
    fclose(hLogFile);

    return(status);     
}

short __declspec(dllexport) WINAPI RP1210_SendCommand( 
                                        short   nCommandNumber,
										short 	nClientID,
										unsigned char *fpchClientCommand,
                                        short	nMessageSize ){
    int status = ERR_DLL_NOT_INITIALIZED;
    if (Xternal_RP1210_SendCommand != NULL){
        status = Xternal_RP1210_SendCommand( nCommandNumber,
                                             nClientID,
                                             fpchClientCommand,
                                             nMessageSize);
    }
    
    /* Make a print of the data bytes*/
    char databytes[200];
    for (int i = 0; i<(3*nMessageSize); i+=3){
        sprintf_s(&databytes[i],3,",%02X", fpchClientCommand[i] );
    }

    sprintf_s(messagelog,sizeof(messagelog),
        "\n%02d,SC,%d,%d,%d",
		nClientID,status,nCommandNumber,nMessageSize);
    fopen_s(&hLogFile, logFileName, "a+");
    fwrite(messagelog, sizeof(char), strlen(messagelog), hLogFile);
    for (int i = 0; i<(nMessageSize); i++){
        sprintf_s(databytes,4,",%02X", fpchClientCommand[i] );
        fwrite(databytes,sizeof(char), strlen(databytes), hLogFile);
    }
    fclose(hLogFile);

    return(status);                 
}

void  __declspec(dllexport) WINAPI RP1210_ReadVersion(
                                        char    *fpchDLLMajorVersion,
										char    *fpchDLLMinorVersion,
										char    *fpchAPIMajorVersion,
                                        char    *fpchAPIMinorVersion ){
    if (Xternal_RP1210_ReadVersion == NULL){
        dll_module = LoadLibrary(xternal_rp1210dll_name);
        Xternal_RP1210_ReadVersion = GetProcAddress(dll_module, "RP1210_ReadVersion");
    }

    if (Xternal_RP1210_ReadVersion != NULL){
        Xternal_RP1210_ReadVersion( fpchDLLMajorVersion,
                                    fpchDLLMinorVersion,
                                    fpchAPIMajorVersion,
                                    fpchAPIMinorVersion );
    }
    else{
        fpchDLLMajorVersion = 0;
        fpchDLLMinorVersion = 0;
        fpchAPIMajorVersion = 0;
        fpchAPIMinorVersion = 0;
    }
}

short __declspec(dllexport) WINAPI RP1210_ReadDetailedVersion( 
                                        short   nClientID,
										char    *fpchAPIVersionInfo,
                                        char    *fpchDLLVersionInfo,
									    char    *fpchFWVersionInfo ){
    short status = 0;

    return(status);
}

short __declspec(dllexport) WINAPI RP1210_GetHardwareStatus(
                                        short   nClientID,
										unsigned char *fpchClientInfo,
                                        short    nInfoSize,
                                        short	 nBlockOnRequest){
    short status = 0;

    return(status);
}

short __declspec(dllexport) WINAPI RP1210_GetHardwareStatusEx(
                                        short   nClientID, 
                                        unsigned char *fpchClientInfo){
    short status = 0;

    return(status);
}

short __declspec(dllexport) WINAPI RP1210_GetErrorMsg(
                                        short   errorCode,
										char    *fpchDescription ){
    short status = 0;

    return(status);

}
short __declspec(dllexport) WINAPI RP1210_GetLastErrorMsg(
                                        short   errorCode,
										int     *SubErrorCode,
										char    *fpchDescription,
										short   nClientID ){
    short status = 0;

    return(status);
}

short __declspec(dllexport) WINAPI RP1210_Ioctl(
                                        short   nClientID,
									    long 	nIoctlID,
									    void	*pInput,
									    void	*pOutput ){
    short status = 0;

    return(status);                                        
}

