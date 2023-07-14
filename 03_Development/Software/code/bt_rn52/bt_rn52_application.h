/**
 * @file    bt_rn52_application.h
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief 
 * @version 0.1
 * @date    2023-05-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _BT_RN52_APPLICATION_H_
#define _BT_RN52_APPLICATION_H_

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

#define BT_UART_TXPIN      8 /* UART1_Tx on GPIO 8 */
#define BT_UART_RXPIN      9 /* UART1_Rx on GPIO 9 */
#define BT_UART_ID         uart1
#define BT_UART_BAUDRATE   115200
#define BT_UART_DATABITS   8
#define BT_UART_STOPBITS   1
#define BT_UART_PARITY     UART_BT_UART_PARITY_NONE

/*** SET COMMANDS V1.16 ***
S-,<text>  - Serialized Name
S|,<hex16> - Audio Settings
-  S^,<dec>   - Auto Power Off timer
S%,<hex16> - Extended Features
SA,<dec>   - Authentication Mode
-  SC,<hex24> - Class of Device
-  SD,<hex8>  - Discovery Mask
-  SF,1       - Factory Default
-  SK,<hex8>  - Connection Mask
-  SM,<hex32> - Microphone Gain
-  SL,<hex32> - Line Input Gain
SN,<text>  - Name
SP,<text>  - Pin Code
SS,<hex8>  - Speaker Level
-  ST,<hex8>  - Tone Level
SU,<hex8>  - UART Baudrate
-  STA,<dec>  - Connection Delay(ms)
-  STP,<dec>  - Pairing Timeout(s)
GB         - AG Battery Status

*** ACTION COMMANDS ***
+          - Disable,Enable Local Echo
@,<0,1>    - Disable,Enable Discovery
#,<0,1>    - Accept,Reject Pairing Passkey
-  $          - Enter DFU Mode
D          - Dump Configuration
K,<hex8>   - Disconnect
-  M,<0,1>    - Mute Active Call
Q          - Connection Status
R,1        - Reboot
-  U          - Reset Paired Device List(PDL)
-  V          - Firmare Version

*** AVRCP COMMANDS ***
AV+        - Volume Up
AV-        - Volume Down
AT+        - Next Track
AT-        - Previous Track
AP         - Play/Pause
AD         - Track Metadata

*** CONNECTION COMMANDS ***
-  A,<text>   - Initiate Voice Call
-  AR         - Redial Last Dialed Number
B          - Reconnect Last Device
-  C          - Accept Incoming Call
-  E          - Drop/Reject Active Or Incoming Call
-  F          - Release All Held Calls
-  J          - Accept Waiting Calls And Release Active Calls
-  L          - Accept Waiting Calls And Hold Active Calls
-  N          - Add Held Call
-  O          - Connect Two Calls And Disconnect The Subscriber
T          - Caller ID Information
-  P          - Initiate Voice Recognition
-  X,<0,1>    - Transfer Call Between HF And AG
*/

/** 
 * Q results:
 * 0 iAP wireless active connection to remote device
 * 1 SPP active connection to remote device
 * 2 A2DP active connection to remote device
 * 3 HFP/HSP active connection to remote device
 * 4 Caller ID notification event from audio gateway
 * 5 Track change event notification
 */

#define RN52_CMD_SIZE_VOLUP   4
#define RN52_CMD_SIZE_VOLDWN  4
#define RN52_CMD_SIZE_NXT     4
#define RN52_CMD_SIZE_PRV     4
#define RN52_CMD_SIZE_PLP     3
#define RN52_CMD_SIZE_TRACK   3  /* Track metadata */
#define RN52_CMD_SIZE_Q       2

#define RN52_GPIO2   28 /* GPIO 28 for rn52's gpio2 notification pin */

typedef enum {
   RN52_CMD_VOLUP = 1,
   RN52_CMD_VOLDWN,
   RN52_CMD_NXT,
   RN52_CMD_PRV,
   RN52_CMD_PLP,
   RN52_CMD_TRACK,
   RN52_CMD_Q,
   /* KEEP AT THE END */
   RN52_CMD_MAX
} RN52_CMD_ID;

typedef struct {
   RN52_CMD_ID cmdId;
   char*       msgPtr;
   uint8_t     msgSize;
} rn52_mcmdMsg;

/**
 * @brief initiate RN52 bluetooth module
 */
void bt_init(void);

/**
 * @brief send an RN52 command to bluetooth module
 */
void bt_sendCommand(RN52_CMD_ID rn52cmd);

inline void bt_sendVolUp(void);
inline void bt_sendVolDown(void);
inline void bt_sendNextTrack(void);
inline void bt_sendPreviousTrack(void);
inline void bt_sendPlayPause(void);

#endif /* _BT_RN52_APPLICATION_H_ */