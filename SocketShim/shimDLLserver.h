#include <windows.h>

#define TXRXSIZE 				4200 

void setupShimSocket(void);

/* Prototypes for RP1210 */
/* These are explicitly defined in the RP1210 API document. */
short __declspec(dllexport) WINAPI RP1210_ClientConnect( 
                                        HWND    hwndClient,
										short 	nDeviceID,
                                       	char    *fpchProtocol,
                                       	long	lTxBufferSize,
                                        long    lRxBufferSize,
                                        short   nIsAppPacketizingInMsgs );

short __declspec(dllexport) WINAPI RP1210_ClientDisconnect( 
                                        short	nClientID );

short __declspec(dllexport) WINAPI RP1210_SendMessage( 
                                        short	nClientID,
                                        unsigned char *fpchClientMessage,
                                        short	nMessageSize,
                                        short   nNotifyStatusOnTx,
                                        short	nBlockOnSend );

short __declspec(dllexport) WINAPI RP1210_ReadMessage(
                                        short   nClientID,
										unsigned char *fpchAPIMessage,
                                        short	nBufferSize,
                                        short	nBlockOnRead );


short __declspec(dllexport) WINAPI RP1210_SendCommand( 
                                        short   nCommandNumber,
										short 	nClientID,
										unsigned char *fpchClientCommand,
                                        short	nMessageSize );

void __declspec(dllexport) WINAPI RP1210_ReadVersion(
                                        char *fpchDLLMajorVersion,
                                        char *fpchDLLMinorVersion,
                                        char *fpchAPIMajorVersion,
                                        char *fpchAPIMinorVersion);
                                    

short __declspec(dllexport) WINAPI RP1210_ReadDetailedVersion( 
                                        short   nClientID,
										char    *fpchAPIVersionInfo,
                                        char    *fpchDLLVersionInfo,
									    char    *fpchFWVersionInfo );

short __declspec(dllexport) WINAPI RP1210_GetHardwareStatus(
                                        short   nClientID,
										unsigned char *fpchClientInfo,
                                        short    nInfoSize,
                                        short	 nBlockOnRequest);

short __declspec(dllexport) WINAPI RP1210_GetHardwareStatusEx(
                                        short   nClientID, 
                                        unsigned char *fpchClientInfo);

short __declspec(dllexport) WINAPI RP1210_GetErrorMsg(
                                        short   errorCode,
										char    *fpchDescription );

short __declspec(dllexport) WINAPI RP1210_GetLastErrorMsg(
                                        short   errorCode,
										int     *SubErrorCode,
										char    *fpchDescription,
										short   nClientID );

short __declspec(dllexport) WINAPI RP1210_Ioctl(
                                        short   nClientID,
									    long 	nIoctlID,
									    void	*pInput,
									    void	*pOutput );

//----------------------------------------------------------------------------------------------------
// RP1210_SendCommand() definitions.
//----------------------------------------------------------------------------------------------------
#define RP1210_Reset_Device 0
#define RP1210_Set_All_Filters_States_to_Pass 3
#define RP1210_Set_Message_Filtering_For_J1939 4
#define RP1210_Set_Message_Filtering_For_CAN 5
#define RP1210_Set_Message_Filtering_For_J1708 7
#define RP1210_Set_Message_Filtering_For_J1850 8
#define RP1210_Set_Message_Filtering_For_ISO15765 9
#define RP1210_Generic_Driver_Command 14
#define RP1210_Set_J1708_Mode 15
#define RP1210_Echo_Transmitted_Messages 16
#define RP1210_Set_All_Filters_States_to_Discard 17
#define RP1210_Set_Message_Receive 18
#define RP1210_Protect_J1939_Address 19
#define RP1210_Set_Broadcast_For_J1708 20
#define RP1210_Set_Broadcast_For_CAN 21
#define RP1210_Set_Broadcast_For_J1939 22
#define RP1210_Set_Broadcast_For_J1850 23
#define RP1210_Set_J1708_Filter_Type 24
#define RP1210_Set_J1939_Filter_Type 25
#define RP1210_Set_CAN_Filter_Type 26
#define RP1210_Set_J1939_shorterpacket_Time 27
#define RP1210_SetMaxErrorMsgSize 28
#define RP1210_Disallow_Further_Connections 29
#define RP1210_Set_J1850_Filter_Type 30
#define RP1210_Release_J1939_Address 31
#define RP1210_Set_ISO15765_Filter_Type 32
#define RP1210_Set_Broadcast_For_ISO15765 33
#define RP1210_Set_ISO15765_Flow_Control 34
#define RP1210_Clear_ISO15765_Flow_Control 35
#define RP1210_Set_J1939_Baud 37
#define RP1210_Set_ISO15765_Baud 38
#define RP1210_Flush_Tx_Rx_Buffers 39
#define RP1210_Set_Broadcast_For_ISO14230 41
#define RP1210_Set_Broadcast_For_ISO9141 42
#define RP1210_Get_Protocol_Connection_Speed 45
#define RP1210_Set_ISO9141ISO14230_Mode 46
#define RP1210_Set_CAN_Baud 47
#define RP1210_Get_Wireless_State 48
#define RP1210_Set_BlockTimeout 215
#define RP1210_Set_J1708_Baud 305
#define RP1210_Set_J1850_Baud 306

//----------------------------------------------------------------------------------------------------
// RP1210 Constants
//----------------------------------------------------------------------------------------------------
#define CONNECTED_WIRED 0 // Adapter is using a wired connection.
#define CONNECTED_WIRELESS 1 // Adapter is using a wireless connection.
#define CONNECTED 1 // Current connection state = Connected
#define NOT_CONNECTED -1 // Current connection state = Disconnected
#define FILTER_PASS_NONE 0 // Current filter state = DISCARD ALL MESSAGES
#define FILTER_PASS_SOME 1 // Current filter state = PASS SOME (some filters set)
#define FILTER_PASS_ALL 2 // Current filter state = PASS ALL
#define NULL_WINDOW 0 // Windows 3.1 is no longer supported.
#define BLOCKING_IO 1 // For Blocking calls to send/read.
#define NON_BLOCKING_IO 0 // For Non-Blocking calls to send/read.
#define BLOCK_INFINITE 0 // For Non-Blocking calls to send/read.
#define CONVERTED_MODE 1 // RP1210Mode="Converted"
#define RAW_MODE 0 // RP1210Mode="Raw"
#define MAX_J1708_MESSAGE_LENGTH 512 // Maximum size a J1708 message can be
#define MAX_J1939_MESSAGE_LENGTH 1795 // Maximum size a J1939 message can be
#define MAX_ISO15765_MESSAGE_LENGTH 4096 // Maximum size an ISO15765 message can be
#define ECHO_OFF 0x00 // EchoMode - do not embed a echo byte in an Rx msg.
#define ECHO_ON 0x01 // EchoMode - embed an echo byte in the Rx msg.
#define RECEIVE_ON 0x01 // Set Message Receive On
#define RECEIVE_OFF 0x00 // Set Message Receive Off
#define ADD_LIST 0x01 // Add a message to the broadcast list.
#define VIEW_B_LIST 0x02 // View an entry in the broadcase list.
#define DESTROY_LIST 0x03 // Remove all entries in the broadcast list.
#define REMOVE_ENTRY 0x04 // Remove a specific entry from the broadcast list.
#define LIST_LENGTH 0x05 // Returns number of items in broadcast list.
#define FILTER_PGN 0x00000001 // Setting of J1939 filters
#define FILTER_SOURCE 0x00000004 // Setting of J1939 filters
#define FILTER_DESTINATION 0x00000008 // Setting of J1939 filters
#define FILTER_INCLUSIVE 0x00 // FilterMode
#define FILTER_EXCLUSIVE 0x01 // FilterMode
#define SILENT_J1939_CLAIM 0x00 // Claim J1939 Address
#define PASS_J1939_CLAIM_MESSAGES 0x01 // Claim J1939 Address
#define BLOCK_UNTIL_DONE 0 // J1939 address claim block...
#define RETURN_BEFORE_COMPLETION 2 // J1939 address claim no need to block...
#define CHANGE_BAUD_NOW 0x00 // Change Baud
#define MSG_FIRST_CHANGE_BAUD 0x01 // Change Baud
#define RP1210_BAUD_9600 0x00 // Change Baud
#define RP1210_BAUD_19200 0x01 // Change Baud
#define RP1210_BAUD_38400 0x02 // Change Baud
#define RP1210_BAUD_57600 0x03 // Change Baud
#define RP1210_BAUD_125k 0x04 // Change Baud
#define RP1210_BAUD_250k 0x05 // Change Baud
#define RP1210_BAUD_500k 0x06 // Change Baud
#define RP1210_BAUD_1000k 0x07 // Change Baud
#define STANDARD_CAN 0x00 // Filters
#define EXTENDED_CAN 0x01 // Filters
#define STANDARD_CAN_ISO15765_EXTENDED 0x02 // 11-bit with ISO15765 extended address
#define EXTENDED_CAN_ISO15765_EXTENDED 0x03 // 29-bit with ISO15765 extended address
#define STANDARD_MIXED_CAN_ISO15765 0x04 // 11-bit identifier with mixed addressing

#define ISO15765_ACTUAL_MESSAGE 0x00 // ISO15765 ReadMessage - type of data
#define ISO15765_CONFIRM 0x01 // ISO15765 ReadMessage - type of data
#define ISO15765_FF_INDICATION 0x02 // ISO15765 ReadMessage - type of data
#define ISO15765_RX_ERROR_INDICATION 0x03 // ISO15765 ReadMessage - Type of data
#define N_OK 0x00 // RP1210_Ioctl()
#define N_TIMEOUT_A 0x01 // RP1210_Ioctl()
#define N_TIMEOUT_Bs 0x02 // RP1210_Ioctl()
#define N_TIMEOUT_C 0x03 // RP1210_Ioctl()
#define N_WRONG_SN 0x04 // RP1210_Ioctl()
#define N_INVALID_FS 0x05 // RP1210_Ioctl()
#define N_UNEXP_PDU 0x06 // RP1210_Ioctl()
#define N_WFT_OVRN 0x07 // RP1210_Ioctl()
#define N_BUFFER_OVERFLOW 0x08 // RP1210_Ioctl()
#define N_ERROR 0x09 // RP1210_Ioctl()
//----------------------------------------------------------------------------------------------------
// RP1210 Return Definitions
//----------------------------------------------------------------------------------------------------
#define NO_ERRORS 0
#define ERR_DLL_NOT_INITIALIZED 128
#define ERR_INVALID_CLIENT_ID 129
#define ERR_CLIENT_ALREADY_CONNECTED 130
#define ERR_CLIENT_AREA_FULL 131
#define ERR_FREE_MEMORY 132
#define ERR_NOT_ENOUGH_MEMORY 133
#define ERR_INVALID_DEVICE 134
#define ERR_DEVICE_IN_USE 135
#define ERR_INVALID_PROTOCOL 136
#define ERR_TX_QUEUE_FULL 137
#define ERR_TX_QUEUE_CORRUPT 138
#define ERR_RX_QUEUE_FULL 139
#define ERR_RX_QUEUE_CORRUPT 140
#define ERR_MESSAGE_TOO_LONG 141
#define ERR_HARDWARE_NOT_RESPONDING 142
#define ERR_COMMAND_NOT_SUPPORTED 143
#define ERR_INVALID_COMMAND 144
#define ERR_TXMESSAGE_STATUS 145
#define ERR_ADDRESS_CLAIM_FAILED 146
#define ERR_CANNOT_SET_PRIORITY 147
#define ERR_CLIENT_DISCONNECTED 148
#define ERR_CONNECT_NOT_ALLOWED 149
#define ERR_CHANGE_MODE_FAILED 150
#define ERR_BUS_OFF 151
#define ERR_COULD_NOT_TX_ADDRESS_CLAIMED 152
#define ERR_ADDRESS_LOST 153
#define ERR_CODE_NOT_FOUND 154
#define ERR_BLOCK_NOT_ALLOWED 155
#define ERR_MULTIPLE_CLIENTS_CONNECTED 156
#define ERR_ADDRESS_NEVER_CLAIMED 157
#define ERR_WINDOW_HANDLE_REQUIRED 158
#define ERR_MESSAGE_NOT_SENT 159
#define ERR_MAX_NOTIFY_EXCEEDED 160
#define ERR_MAX_FILTERS_EXCEEDED 161
#define ERR_HARDWARE_RP1210_STATUS_CHANGE 162
#define ERR_INI_FILE_NOT_IN_WIN_DIR 202
#define ERR_INI_SECTION_NOT_FOUND 204
#define ERR_INI_KEY_NOT_FOUND 205
#define ERR_INVALID_KEY_STRING 206
#define ERR_DEVICE_NOT_SUPPORTED 207
#define ERR_INVALID_PORT_PARAM 208
#define ERR_COMMAND_TIMED_OUT 213
#define ERR_OS_NOT_SUPPORTED 220
#define ERR_COMMAND_QUEUE_IS_FULL 222
#define ERR_CANNOT_SET_CAN_BAUDRATE 224
#define ERR_CANNOT_CLAIM_BROADCAST_ADDRESS 225
#define ERR_OUT_OF_ADDRESS_RESOURCES 226
#define ERR_ADDRESS_RELEASE_FAILED 227
#define ERR_COMM_DEVICE_IN_USE 230
#define ERR_DATA_LINK_CONFLICT 441
#define ERR_ADAPTER_NOT_RESPONDING 453
#define ERR_CAN_BAUD_SET_NONSTANDARD 454
#define ERR_MULTIPLE_CONNECTIONS_NOT_ALLOWED_NOW 455
#define ERR_J1708_BAUD_SET_NONSTANDARD 456
#define ERR_J1939_BAUD_SET_NONSTANDARD 457
#define ERR_ISO15765_BAUD_SET_NONSTANDARD 458
#define ERR_INVALID_IOCTL_ID 600
#define ERR_NULL_PARAMETER 601
#define ERR_HARDWARE_NOT_SUPPORTED 602
#define ERR_CANNOT_DETERMINE_BAUD_RATE 603

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long UL;

//----------------------------------------------------------------------------------------------------
// RP1210 Function Type Definitions
//----------------------------------------------------------------------------------------------------
/***********************/
/* function Types from RP1210 manual*/
/***********************/
typedef short  (WINAPI* CLIENTCONNECT) (HWND, short, char*, long, long, short);
typedef short  (WINAPI* CLIENTDISCONNECT) (short);
typedef short  (WINAPI* SENDMESSAGE)(short, unsigned char*, short, short, short);
typedef short  (WINAPI* READMESSAGE)(short, unsigned char*, short, short);
typedef short  (WINAPI* SENDCOMMAND)(short, short, unsigned char*, short);
typedef void   (WINAPI* READVERSION)(char *, char *, char *, char *);
typedef short  (WINAPI* GETERRORMESSAGE)(short, unsigned char*);
typedef short  (WINAPI* GETLASTERRORMESSAGE)(short, int *, char *, short);
typedef short  (WINAPI* GETHARDWARESTATUS)(short, unsigned char*, short, short);
typedef short  (WINAPI* GETHARDWARESTATUSEX)(short, unsigned char*);
typedef short  (WINAPI* READDETAILEDVERSION)(short, char *, char *, char *);
typedef short  (WINAPI* IOCTL)(short, long, void *, void *);

