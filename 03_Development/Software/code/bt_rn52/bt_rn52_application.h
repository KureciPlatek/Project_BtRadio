/**
 * @file    bt_rn52_application.h
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief   Driver for RN52 Bluetooth module
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
#define BT_UART_PARITY     UART_PARITY_NONE

/* All commands are described in bt_rn52_commandDescription.md file */
#define RN52_CMD_SIZE_VOLUP   4
#define RN52_CMD_SIZE_VOLDWN  4
#define RN52_CMD_SIZE_NXT     4
#define RN52_CMD_SIZE_PRV     4
#define RN52_CMD_SIZE_PLP     3
#define RN52_CMD_SIZE_TRACK   3  /* Track metadata */
#define RN52_CMD_SIZE_Q       2

#define RN52_QREPLY_TRACK_POS 5
#define RN52_QREPLY_CONNMASK  0x0F

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
 * 
 * @param rn52cmd [in] command defined in enum RN52_CMD_ID
 */
void bt_sendCommand(RN52_CMD_ID rn52cmd);

/**
 * @brief process bluetooth last inputs.
 * It is like the main function of this module which hast to be called
 * periodically.
 */
void bt_processInputs(void);

/**
 * @brief Handle notification interrupt from RN52 module
 */
void rn52_handleGpio2(void);

#endif /* _BT_RN52_APPLICATION_H_ */