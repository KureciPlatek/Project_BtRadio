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

#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bt_rn52_application.h"
#include "ep_application.h"
#include "hal_main.h"

#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

#define MAX_RN52_INPUTBUFF_LINES       10 /* Maximum lines for input buffer (data coming from rn52 through uart) */
#define MAX_RN52_INPUTBUFF_CHARACTERS  50 /* Maximum characters for input buffer (data coming from rn52 through uart) */

#define RN52_TIMEOUT_CLICKS_MS   20

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
char rn52_inputBuffer[MAX_RN52_INPUTBUFF_LINES][MAX_RN52_INPUTBUFF_CHARACTERS];

/**
 * @brief Receive RX on bluetooth's module UART
 */
static void bt_irqUartRx(void);

static void rn52_processReply(void);

static void rn52_processQReply(void);

static void rn52_processADReply(uint8_t line);

static void rn52_cmdProcessed(void);

static uint8_t convertToHex(char character);

int64_t alarm_callback_rn52(alarm_id_t id, void *user_data);

static void rn52_dropLine(uint8_t lineToDrop);

/**
 * @brief elements for state machine to process command send and answer process from rn52
 */
static uint8_t _indexLine;
static uint8_t _indexChar;

/* RE timeout between clicks. If fired, cancel actual rotation calculation */
static alarm_id_t currentAlarm_rn52;
static bool       _eventToProcess;

int64_t alarm_callback_rn52(alarm_id_t id, void *user_data)
{
   currentAlarm_rn52 = 0x00;
   _eventToProcess = true;
   return 0;
}

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

   _indexLine = 0x00;
   _indexChar = 0x00;
   _eventToProcess = false;

   currentAlarm_rn52 = (alarm_id_t)0x00;

   printf("[BT][API] Init BT done\n");
}

void bt_deactivate(void)
{
   int UART_IRQ = BT_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
   irq_set_enabled(UART_IRQ, false);
   hal_deactivateBT();
}

void bt_activate(void)
{
   int UART_IRQ = BT_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
   irq_set_enabled(UART_IRQ, true);
   hal_activateBT();
}

void bt_sendCommand(RN52_CMD_ID rn52cmd)
{
   if(RN52_CMD_MAX <= rn52cmd )
   {
      /* Not a reckognized command */
      printf("[BT][ERROR] wrong RN52 command 0x%2x\n", (uint8_t)rn52cmd);
   }
   else
   {
      uart_write_blocking(BT_UART_ID, (uint8_t*) allRN52_cmd[rn52cmd].msgPtr, allRN52_cmd[rn52cmd].msgSize);
   }
}

/**
 * @brief In case of GPIO2 triggered, send Q cmd and process reply
 * 
 */
void rn52_handleGpio2(void)
{
//   printf("[BT][API]GPIO 2 IRQ\n");
   /* Wait 10 ms before asking what happened */
   /* send Q to RN52 module */
   bt_sendCommand(RN52_CMD_Q);
   currentAlarm_rn52 = add_alarm_in_ms(RN52_TIMEOUT_CLICKS_MS, alarm_callback_rn52, NULL, false);
}

/**
 * @brief Called by main to check if there are things to do
 * 
 */
void bt_processInputs(void)
{
   if(true == _eventToProcess)
   {
      rn52_processReply();
      _eventToProcess = false;
   }
}

//////////////////// Bluetooth and rn52 specific functions (statics) ////////////////////

/**
 * @brief UART IRQ for one character coming from RN52.
 * RN52 sends reply as string of characters ended by a '\r'
 * character.
 * Sometimes it is one hexadecimal line, sometimes strings
 * of ASCII chars on many lines, all ended by '\r'.
 * 
 * As the aim is to process this data peacefully, save it first
 * in a temporar buffer.
 * As soon as a '\r' char is received, fire a timer of 10 ms to
 * wait if there are no other chars coming.
 * If IRQ called again while timer runs, kill timer
 * 
 * Received messages from RN52 is sometimes polluted with \r and \n 
 * characters, which do disturb getting distincts answers.
 * If one of them is received, drop it.
 * \r = 13
 * \n = 10
 */
static void bt_irqUartRx(void) 
{
   uint8_t ch = uart_getc(BT_UART_ID);
#if 0
   if(('\r' == ch) || ('\n' == ch))
   {
      printf("\\%d", (uint8_t)ch);
   }
   else
   {
      printf("%c", ch);
   }
#endif
#if 1
   /* If \r char, drop it as it pollutes communication. 
   It is sometimes received two or 3 times per line */
   if(('\r' != ch)
    &&(0x00 != (uint8_t)ch))
   {
      /* New character, cancel timer if activated */
      if(0x00 != currentAlarm_rn52)
      {
         cancel_alarm(currentAlarm_rn52);
      }

      /* Save reply only if we are awaiting for one */
      if(MAX_RN52_INPUTBUFF_LINES > _indexLine)
      {
         if(MAX_RN52_INPUTBUFF_CHARACTERS > _indexChar)
         {
            rn52_inputBuffer[_indexLine][_indexChar] = ch;
            _indexChar++;
         }

         /* New line if too many characters for input buffer or \n char received
         look only at \n chars, as \r may be received multiple times for one reply line */
         if((MAX_RN52_INPUTBUFF_CHARACTERS == _indexChar) || ('\n' == ch))
         {
            /* Prepare new line in case of other characters are coming */
            _indexLine++;
            _indexChar = 0x00;
            /* Fire alarm to process received reply */
            currentAlarm_rn52 = add_alarm_in_ms(RN52_TIMEOUT_CLICKS_MS, alarm_callback_rn52, NULL, false);
         }
      }
      else
      {
         /* Loo buffer */
         _indexLine = 0x00;
      }
   }
#endif   
}

/**
 * @brief Called by timer, which means one or many characters were received
 * and that we are currently in a peaceful moment (no new characters since 
 * RN52_TIMEOUT_CLICKS_MS ms)
 */
static void rn52_processReply(void)
{
   uint8_t index = 0x00;
   /* drop polluting lines */
   for(index = 0; index < MAX_RN52_INPUTBUFF_LINES; index++)
   {
      /* Process what kind of reply we had. Horrible way but RN52 sends sometimes random replies */
      if (('A' == rn52_inputBuffer[index][0])
       && ('O' == rn52_inputBuffer[index][1])
       && ('K' == rn52_inputBuffer[index][2]))
      {
         rn52_dropLine(index);
      }
      else
      {
         if (('T' == rn52_inputBuffer[index][0])
          && ('i' == rn52_inputBuffer[index][1])
          && ('t' == rn52_inputBuffer[index][2])
          && ('l' == rn52_inputBuffer[index][3])
          && ('e' == rn52_inputBuffer[index][4])
          && ('=' == rn52_inputBuffer[index][5]))
         {
            rn52_processADReply(index);
         }

         /* Check that reply is not "AOK\r", but something like "XXXX\r" where X are hexadecimal numbers
          * AOK is replied by all AD/AV commands, like next track, play pause, etc... 
          */
         if ('\n'== rn52_inputBuffer[index][4])
         {
            /* Check if it is an AOK from RN52 (then discard) */
            rn52_processQReply();
         }
      }
   }
}

static void rn52_processQReply(void)
{
   /* Convert values first */
   uint8_t byte0low  = 0x00;
   uint8_t byte0high = 0x00;
   uint8_t resultByte0 = 0x00;   /* Only byte0 bit 5 is interessant for us */

   uint8_t connectionStatus  = 0x00;

   byte0low  = convertToHex(rn52_inputBuffer[0][3]);
   byte0high = convertToHex(rn52_inputBuffer[0][2]) << 1;

   resultByte0 = byte0high | byte0low;
   printf("[BT][DBG] Q received = 0x%2x\n", resultByte0);

   /* Should be a 2 bytes reply on one line. Byte[0].bit_1 should be set to notify track change event */
   if(0 < (0x10 && resultByte0))
   {
      printf("[BT][CMD] Track change notification\n");

      /* Data processed, clean input buffer from RN52 */
      rn52_cmdProcessed();

      /* Send new cmd: ask what happened */
      bt_sendCommand(RN52_CMD_TRACK);  

      /* Let time before processing answer */
      currentAlarm_rn52 = add_alarm_in_ms(RN52_TIMEOUT_CLICKS_MS, alarm_callback_rn52, NULL, false);
   }

   /* Other kind of reply could be processed here depending on Q reply (like "new connection")*/
   connectionStatus = (resultByte0 & 0x0F);
   if(connectionStatus >= 1)
   {
      printf("[BT][CMD] Connected to a device: 0x%2x\n", connectionStatus);
   }

}

/**
 * @brief 
 * @optimisation: change copy of values to a shifted pointer of 6 oder 7 chars
 * @param line 
 */
static void rn52_processADReply(uint8_t line)
{
   printf("[BT][DBG] Process track change at line %d\n", line);
   /* Kind of reply we may have when asking for track metadata:
    *  Title=Veterstift
    *  Artist=Hush
    *  Album=Ruimtevaart EP
    */
   uint8_t index     = 0x00;
   uint8_t indexChar = 0x00;
   uint8_t track[EPAPER_CHARS_PER_LINE];  /* Max EP characters per line */
   uint8_t artist[EPAPER_CHARS_PER_LINE]; /* Max EP characters per line */
   uint8_t Album[EPAPER_CHARS_PER_LINE]; /* Max EP characters per line */

   for(index = 0; index < EPAPER_CHARS_PER_LINE; index++)
   {
      track[index]  = 0x00;
      artist[index] = 0x00;
      Album[index]  = 0x00;
   }

   /* ==================== PRINT TITLE ==================== */
   /* Drop all 0x00 if exists */
   /* +6 is for "Title=" character offset which we don't require */
   do {
      track[indexChar] = rn52_inputBuffer[line][indexChar+6];
      indexChar++;
   }
   while(('\n' != rn52_inputBuffer[line][indexChar+6])
    && (EPAPER_CHARS_PER_LINE > (indexChar+6)));

   ep_write(EPAPER_PLACE_BT_TRACK, 1, (char *)&track[0], false);


   /* ==================== PRINT ARTIST ==================== */
   /* +7 is for "Artist=" character offset which we don't require */
   indexChar = 0x00;
   while(('\n' != rn52_inputBuffer[line+1][indexChar+7])
    &&   (EPAPER_CHARS_PER_LINE > (indexChar+7)))
   {
      artist[indexChar] = rn52_inputBuffer[line+1][indexChar+7];
      indexChar++;
   }
   ep_write(EPAPER_PLACE_BT_TRACK, 3, (char *)&artist[0], false);


   /* ==================== PRINT ALBUM ==================== */
   /* +6 is for "Album=" character offset which we don't require */
   indexChar = 0x00;
   while(('\n' != rn52_inputBuffer[line+2][indexChar+6])
    &&   (EPAPER_CHARS_PER_LINE > (indexChar+6)))
   {
      Album[indexChar] = rn52_inputBuffer[line+2][indexChar+6];
      indexChar++;
   }
   ep_write(EPAPER_PLACE_BT_TRACK, 5, (char *)&Album[0], false);

   /* Write down to display */
   ep_flush();

   /* Finished with cmd process */
   rn52_cmdProcessed();
}

static void rn52_cmdProcessed(void)
{
   uint8_t indexLines = 0x00;

   /* Reset input buffer */
   for(indexLines = 0x00; indexLines < MAX_RN52_INPUTBUFF_LINES; indexLines++)
   {
      rn52_dropLine(indexLines);
   }
   _indexLine = 0x00;
   _indexChar = 0x00;
}

static void rn52_dropLine(uint8_t lineToDrop)
{
   printf("[BT][DBG] clean line\n");
   uint8_t indexChar = 0x00;
   for(indexChar = 0x00; indexChar < MAX_RN52_INPUTBUFF_CHARACTERS; indexChar++)
   {
      rn52_inputBuffer[lineToDrop][indexChar] = 0x00;
   }
   _indexChar = 0x00;
}

/**
 * @brief convert char to hexadecimal value.
 */
static uint8_t convertToHex(char character)
{
   uint8_t retHex = 0x00;

   /* Characters only from h0 to hF, others are discarded */
   switch (character)
   {
   case '0':
      retHex = 0x00;
      break;
   case '1':
      retHex = 0x01;
      break;
   case '2':
      retHex = 0x02;
      break;
   case '3':
      retHex = 0x03;
      break;
   case '4':
      retHex = 0x04;
      break;
   case '5':
      retHex = 0x05;
      break;
   case '6':
      retHex = 0x06;
      break;
   case '7':
      retHex = 0x07;
      break;
   case '8':
      retHex = 0x08;
      break;
   case '9':
      retHex = 0x09;
      break;
   case 'A':
      retHex = 0x0A;
      break;
   case 'B':
      retHex = 0x0B;
      break;
   case 'C':
      retHex = 0x0C;
      break;
   case 'D':
      retHex = 0x0D;
      break;
   case 'E':
      retHex = 0x0E;
      break;
   case 'F':
      retHex = 0x0F;
      break;
   
   default:
      break;
   }

   return retHex;
}
