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

#define MAX_RN52_INPUTBUFF_LINES       6  /* Maximum lines for input buffer (data coming from rn52 through uart) */
#define MAX_RN52_INPUTBUFF_CHARACTERS  20 /* Maximum characters for input buffer (data coming from rn52 through uart) */


/* Expected replies from RN52 */
static uint8_t qSent;
static uint8_t trackMetadataSent;
static uint8_t token_Receivedrn52Msg;

/* Bluetooth RN52 command messages (V1.16) */
static char rn52_cmd_volup[RN52_CMD_SIZE_VOLUP]   = {'A','V','+','\r'}; //  - Volume Up
static char rn52_cmd_voldwn[RN52_CMD_SIZE_VOLDWN] = {'A','V','-','\r'}; //  - Volume Down
static char rn52_cmd_nxt[RN52_CMD_SIZE_NXT]       = {'A','T','+','\r'}; //  - Next Track
static char rn52_cmd_prv[RN52_CMD_SIZE_PRV]       = {'A','T','-','\r'}; //  - Previous Track
static char rn52_cmd_plp[RN52_CMD_SIZE_PLP]       = {'A','P','\r'};     //  - Play/Pause
static char rn52_cmd_track[RN52_CMD_SIZE_TRACK]   = {'A','D','\r'};     //  - REquest track metadata
static char rn52_cmd_q[RN52_CMD_SIZE_Q]           = {'Q','\r'};         //  - Querry info

/* Bluetooth RN52 command messages (V1.16) */
static rn52_mcmdMsg allRN52_cmd[RN52_CMD_MAX] = {
   {RN52_CMD_VOLUP,  &rn52_cmd_volup[0],  RN52_CMD_SIZE_VOLUP  },
   {RN52_CMD_VOLDWN, &rn52_cmd_voldwn[0], RN52_CMD_SIZE_VOLDWN },
   {RN52_CMD_NXT,    &rn52_cmd_nxt[0],    RN52_CMD_SIZE_NXT    },
   {RN52_CMD_PRV,    &rn52_cmd_prv[0],    RN52_CMD_SIZE_PRV    },
   {RN52_CMD_PLP,    &rn52_cmd_plp[0],    RN52_CMD_SIZE_PLP    },
   {RN52_CMD_PLP,    &rn52_cmd_track[0],  RN52_CMD_SIZE_TRACK  },
   {RN52_CMD_Q,      &rn52_cmd_q[0],      RN52_CMD_SIZE_Q      }
};

/**
 * @brief save characters coming from rn52's uart. Save them in a buffer.
 * As this data comes with \r characters in it, use it to save a new line
 * Or if data line received is too long for MAX_RN52_INPUTBUFF_CHARACTERS,
 * add a line break
 */
uint8_t rn52_inputBuffer[MAX_RN52_INPUTBUFF_LINES][MAX_RN52_INPUTBUFF_CHARACTERS];

/**
 * @brief Handle notification interrupt from RN52 module
 */
static void rn52_handleGpio2(void);
/**
 * @brief Receive RX on bluetooth's module UART
 */
static void bt_irqUartRx(void);


void bt_init(void)
{
   qSent = 0x00;
   trackMetadataSent = 0x00;
   token_Receivedrn52Msg = 0x00;
   
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

   gpio_set_irq_enabled(RN52_GPIO2, GPIO_IRQ_EDGE_FALL, true); //monitor pin 1 connected to pin 0
   gpio_add_raw_irq_handler(RN52_GPIO2, rn52_handleGpio2);
}

void bt_sendCommand(RN52_CMD_ID rn52cmd)
{
   if(RN52_CMD_MAX >= rn52cmd )
   {
      /* Not a reckognized command */
      printf("RN52 command error\n");
   }
   else
   {
      uart_write_blocking(BT_UART_ID, (uint8_t*) allRN52_cmd[rn52cmd].msgPtr, allRN52_cmd[rn52cmd].msgSize);
   }
}

void bt_processInputs(void)
{
   uint8_t qValue = 0x00;
   uint8_t index = 0x00;

   if(0x01 == token_Receivedrn52Msg)
   {
      token_Receivedrn52Msg = 0x00;
      if(0x01 == qSent)
      {  
         /* Process of first byte: */
         qValue = rn52_inputBuffer[0][0];
         if(0x01 >= (qValue >> RN52_QREPLY_TRACK_POS))  /* Track metadata change? */
         {
            bt_sendCommand(RN52_CMD_TRACK);  /* Ask for it */
         }

         qValue = rn52_inputBuffer[1][0];
         if(0x03 == (qValue & RN52_QREPLY_CONNMASK)) /* bytes 0-3 of byte 1 are to be analyzed per value and not bitwise */
         {
            ep_write(EP_PLACE_CONNECTION, 0, "Bluetooth connected");
         }
         qSent = 0x00;
      }

      /* Print track metadata */
      if(0x01 == trackMetadataSent)
      {
         for(index = 0x00; index < MAX_RN52_INPUTBUFF_LINES; index++)
         {
            ep_write(EP_PLACE_TRACK, index, &rn52_inputBuffer[index][0]);
         }
      }
   }
}

//////////////////// Bluetooth and rn52 specific functions (statics) ////////////////////

static void bt_irqUartRx(void) 
{
   uint8_t ch = uart_getc(BT_UART_ID);
   printf("%c", ch);

   while (uart_is_readable(BT_UART_ID))
   {
      uint8_t indexLine = 0x00;
      uint8_t indexChar = 0x00;

      if(MAX_RN52_INPUTBUFF_LINES > indexLine)
      {
         if(MAX_RN52_INPUTBUFF_CHARACTERS > indexChar)
         {
            rn52_inputBuffer[indexLine][indexChar] = (uint8_t)ch;
            indexChar++;
         }

         if((MAX_RN52_INPUTBUFF_CHARACTERS == indexChar)
         || ('\r' == ch))
         {
            indexLine;
         }
      }

      ch = uart_getc(BT_UART_ID);

//      printf("%c", ch);
      /* @todo work received reply and take a decision about it */
   }

   token_Receivedrn52Msg = 0x01;
}

static void rn52_handleGpio2(void)
{
   if (gpio_get_irq_event_mask(RN52_GPIO2) & GPIO_IRQ_EDGE_FALL)
   {
      /* send Q to RN52 module */
      bt_sendCommand(RN52_CMD_Q);
   }
}
