/**
 * @file    bt_rn52_application.c
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief 
 * @version 0.1
 * @date    023-05-26
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "bt_rn52_application.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

/* Rotary encoder states */
static STATE_RE state_RE1;
static STATE_RE state_RE2;

/* Bluetooth RN52 command messages (V1.16) */
static char rn52_cmd_volup[RN52_CMD_SIZE_VOLUP]   = {'A','V','+','\r'}; //  - Volume Up
static char rn52_cmd_voldwn[RN52_CMD_SIZE_VOLDWN] = {'A','V','-','\r'}; //  - Volume Down
static char rn52_cmd_nxt[RN52_CMD_SIZE_NXT]       = {'A','T','+','\r'}; //  - Next Track
static char rn52_cmd_prv[RN52_CMD_SIZE_PRV]       = {'A','T','-','\r'}; //  - Previous Track
static char rn52_cmd_plp[RN52_CMD_SIZE_PLP]       = {'A','P','\r'};     //  - Play/Pause

/* Bluetooth RN52 command messages (V1.16) */
static rn52_mcmdMsg allRN52_cmd[RN52_CMD_MAX] = {
   {RN52_CMD_VOLUP,  &rn52_cmd_volup[0],  RN52_CMD_SIZE_VOLUP  },
   {RN52_CMD_VOLDWN, &rn52_cmd_voldwn[0], RN52_CMD_SIZE_VOLDWN },
   {RN52_CMD_NXT,    &rn52_cmd_nxt[0],    RN52_CMD_SIZE_NXT    },
   {RN52_CMD_PRV,    &rn52_cmd_prv[0],    RN52_CMD_SIZE_PRV    },
   {RN52_CMD_PLP,    &rn52_cmd_plp[0],    RN52_CMD_SIZE_PLP    }
};

void bt_init(void)
{
   // Set up our UART with a basic baud rate.
   uart_init(BT_UART_ID, BT_UART_BAUDRATE);

   // Set the TX and RX pins by using the function select on the GPIO
   // Set datasheet for more information on function select
   gpio_set_function(BT_UART_TXPIN, GPIO_FUNC_UART);
   gpio_set_function(BT_UART_RXPIN, GPIO_FUNC_UART);

   // Actually, we want a different speed
   // The call will return the actual baud rate selected, which will be as close as
   // possible to that requested
//   int __unused actual = uart_set_baudrate(BT_UART_ID, BT_UART_BAUDRATE);

   // Set UART flow control CTS/RTS
   uart_set_hw_flow(BT_UART_ID, false, false);

   // Set our data format
   uart_set_format(BT_UART_ID, BT_UART_DATABITS, BT_UART_STOPBITS, BT_UART_PARITY);

   // Turn off FIFO's - we want to do this character by character
   uart_set_fifo_enabled(BT_UART_ID, false);

   // Set up a RX interrupt
   // We need to set up the handler first
   // Select correct interrupt for the UART we are using
   int UART_IRQ = BT_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

   // And set up and enable the interrupt handlers
   irq_set_exclusive_handler(UART_IRQ, bt_irqUartRx);
   irq_set_enabled(UART_IRQ, true);

   // Now enable the UART to send interrupts - RX only
   uart_set_irq_enables(BT_UART_ID, true, false);
}


void rn52_send(uint8_t * msg, uint8_t len)
{
   uart_write_blocking (BT_UART_ID, msg, len);
}


// RX interrupt handler
void bt_irqUartRx(void) 
{
   uint8_t ch = uart_getc(BT_UART_ID);
   printf("%c", ch);

   while (uart_is_readable(BT_UART_ID))
   {
      ch = uart_getc(BT_UART_ID);
      printf("%c", ch);

      /* @todo print on screen */
   }
}

inline void bt_sendVolUp(void)
{
   rn52_send((uint8_t*) allRN52_cmd[RN52_CMD_VOLUP].msgPtr, allRN52_cmd[RN52_CMD_VOLUP].msgSize);
}

inline void bt_sendVolDown(void)
{
   rn52_send((uint8_t*) allRN52_cmd[RN52_CMD_VOLDWN].msgPtr, allRN52_cmd[RN52_CMD_VOLDWN].msgSize);
}

inline void bt_sendNextTrack(void)
{
   rn52_send((uint8_t*) allRN52_cmd[RN52_CMD_NXT].msgPtr, allRN52_cmd[RN52_CMD_NXT].msgSize);
}

inline void bt_sendPreviousTrack(void)
{
   rn52_send((uint8_t*) allRN52_cmd[RN52_CMD_PRV].msgPtr, allRN52_cmd[RN52_CMD_PRV].msgSize);
}

inline void bt_sendPlayPause(void)
{
   rn52_send((uint8_t*) allRN52_cmd[RN52_CMD_PLP].msgPtr, allRN52_cmd[RN52_CMD_PLP].msgSize);
}

