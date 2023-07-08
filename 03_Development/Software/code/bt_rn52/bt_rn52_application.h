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

/*** SET COMMANDS V1.10 ***
S-,<text>  - Serialized Name
S|,<hex8>  - Audio Route
S^,<dec>   - Auto Power Off timer
S%,<hex8>  - Extended features
SA,<dec>   - Authentication mode
SC,<hex24> - Class of Device
SD,<hex8>  - Discovery Mask
SF,1       - Factory Default
SK,<hex8>  - Connection Mask
SN,<text>  - Name
SP,<text>  - Pin Code
                                                                             
*** ACTION COMMANDS ***                                                      
@,<0,1>    - Disable,Enable Discovery                                        
D          - Dump Configuration                                              
K,<hex8>   - Disconnect                                                      
Q          - Connection Status
R,1        - Reboot
V          - Firmare Version

*** AVRCP COMMANDS ***
AV+        - Volume UP
AV-        - Volume Down
AT+        - Next Track
AT-        - Previous Track
AP         - Play/Pause

*** CONNECTION COMMANDS ***
A,<text>   - Initiate a voice call
B          - Reconnect to last device
C          - Accept incoming call
E          - Drop/Reject active or incoming call

*** GPIO COMMANDS ***
I@         - Show GPIO input/output mask
I@,<hex16> - Set GPIO input/output mask
I&         - Show GPIO input/output state
I&,<hex16> - Set GPIO output state
*/


#define RN52_CMD_SIZE_VOLUP   4
#define RN52_CMD_SIZE_VOLDWN  4
#define RN52_CMD_SIZE_NXT     4
#define RN52_CMD_SIZE_PRV     4
#define RN52_CMD_SIZE_PLP     3

typedef enum {
   RN52_CMD_VOLUP = 1,
   RN52_CMD_VOLDWN,
   RN52_CMD_NXT,
   RN52_CMD_PRV,
   RN52_CMD_PLP,
   /* KEEP AT THE END */
   RN52_CMD_MAX
} RN52_CMD_ID;

typedef struct {
   RN52_CMD_ID cmdId;
   char* msgPtr;
   uint8_t msgSize;
} rn52_mcmdMsg;

#define UART_TX_PIN 8 /* UART1_Tx on GPIO 8 */
#define UART_RX_PIN 9 /* UART1_Rx on GPIO 9 */

#define UART_ID   uart1
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

/* Rotary encoders */
/* Rotary encoder 1 */
#define RE1_A_CLK 17 /* GPIO17, pin 22 */
#define RE1_B_DT  16 /* GPIO16, pin 21 */
#define RE1_SW_SW 18 /* GPIO18, pin 24 */
/* Rotary encoder 2 */
#define RE2_A_CLK 21 /* GPIO21, pin 27 */
#define RE2_B_DT  20 /* GPIO20, pin 26 */
#define RE2_SW_SW 19 /* GPIO19, pin 25 */

typedef enum {
   STATE_RE_WAIT_A,
   STATE_RE_WAIT_B,
   STATE_RE_IDLE
} STATE_RE;

void bt_init(void);

void bt_init_re(void);

void bt_send(uint8_t * msg, uint8_t len);

void on_uart_rx(void);

void bt_processEvents(void);

//void gpio_callback(uint gpio, uint32_t events);

#endif /* _BT_RN52_APPLICATION_H_ */