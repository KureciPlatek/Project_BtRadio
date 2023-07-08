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

/* Events tokens. Basic way to do it, but asynchronize IRQ with event forwarding to RN52 module */
static uint8_t bt_event_volup;
static uint8_t bt_event_voldwn;
static uint8_t bt_event_next;
static uint8_t bt_event_prev;
static uint8_t bt_event_plps;

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


static void alarm_irq(void) 
{
   // Clear the alarm irq
   hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);

   // Assume alarm 0 has fired
   printf("Alarm IRQ fired\n");
   state_RE1 = STATE_RE_IDLE;
}

static void alarm_in_us(uint32_t delay_us) 
{
   hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
   irq_set_exclusive_handler(ALARM_IRQ, alarm_irq);
   irq_set_enabled(ALARM_IRQ, true);
   uint64_t target = timer_hw->timerawl + delay_us;
   timer_hw->alarm[ALARM_NUM] = (uint32_t) target;
}


/**
 * @brief GPIO interrupt on RE
 * 
 * @param gpio 
 * @param events 
 */
void gpio_rn52_callback(uint gpio, uint32_t events) 
{
   printf("GPIO Interrupt %d\n", gpio);
   /* --------- ROTARY ENCODER 1 --------- */
   switch(state_RE1)
   {
      case STATE_RE_IDLE:
      {
         if(RE1_A_CLK == gpio) {
            state_RE1 = STATE_RE_WAIT_B;
            alarm_in_us(150000);    // Start delay before considering B event as void.
         }
         if(RE1_B_DT == gpio) {
            state_RE1 = STATE_RE_WAIT_A;
         }
      }
         break;
      case STATE_RE_WAIT_B:
      {
         if(RE1_B_DT == gpio) {
            /* Clockwise on RE1 confirmed -> send vol+ to RN52 */
            printf("Vol+\n");
            bt_event_volup = 0x01;
            /* Ask also for SS,<hex8>  - Speaker Level */
            state_RE1 = STATE_RE_IDLE;
         }
      }
         break;
      case STATE_RE_WAIT_A:
      {
         if(RE1_A_CLK == gpio) {
            /* Anticlockwise on RE1 confirmed -> send vol- to RN52 */
            printf("Vol-\n");
            bt_event_voldwn = 0x01;
            /* Ask also for SS,<hex8>  - Speaker Level */
            state_RE1 = STATE_RE_IDLE;
         }
      }
         break;
      default:
         break;
   }

   /* --------- ROTARY ENCODER 2 --------- */
   switch(state_RE2)
   {
      case STATE_RE_IDLE:
      {
         if(RE2_A_CLK == gpio) {
            state_RE2 = STATE_RE_WAIT_B;
         }
         if(RE2_B_DT == gpio) {
            state_RE2 = STATE_RE_WAIT_A;
         }
      }
         break;
      case STATE_RE_WAIT_B:
      {
         if(RE2_B_DT == gpio) {
            /* Clockwise on RE2 confirmed -> send next_track to RN52 */
            printf("Next\n");
            bt_event_next = 0x01;
            state_RE2 = STATE_RE_IDLE;
         }
      }
         break;
      case STATE_RE_WAIT_A:
      {
         if(RE2_A_CLK == gpio) {
            /* Anticlockwise on RE2 confirmed -> send prev_track to RN52 */
            printf("Prev\n");
            bt_event_prev = 0x01; 
            state_RE2 = STATE_RE_IDLE;
         }
      }
         break;
      default:
         break;
   }

   if((RE1_SW_SW == gpio) || (RE2_SW_SW == gpio))
   {
      /* Toggle play/pause on RN52 */
      printf("PlayPoz\n");
      bt_event_plps = 0x01;
   }


    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
//    gpio_event_string(event_str, events);
    printf("GPIO %d \n", gpio);
}

void bt_init(void)
{
   bt_event_volup = 0x00;
   bt_event_voldwn = 0x00;
   bt_event_next = 0x00;
   bt_event_prev = 0x00;
   bt_event_plps = 0x00;

   // Set up our UART with a basic baud rate.
   uart_init(UART_ID, BAUD_RATE);

   // Set the TX and RX pins by using the function select on the GPIO
   // Set datasheet for more information on function select
   gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
   gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

   // Actually, we want a different speed
   // The call will return the actual baud rate selected, which will be as close as
   // possible to that requested
//   int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

   // Set UART flow control CTS/RTS
   uart_set_hw_flow(UART_ID, false, false);

   // Set our data format
   uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

   // Turn off FIFO's - we want to do this character by character
   uart_set_fifo_enabled(UART_ID, false);

   // Set up a RX interrupt
   // We need to set up the handler first
   // Select correct interrupt for the UART we are using
   int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

   // And set up and enable the interrupt handlers
   irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
   irq_set_enabled(UART_IRQ, true);

   // Now enable the UART to send interrupts - RX only
   uart_set_irq_enables(UART_ID, true, false);

   bt_init_re();
}

void bt_init_re(void)
{
   /* RE State machines */
   state_RE1 = STATE_RE_IDLE;
   state_RE2 = STATE_RE_IDLE;

   gpio_set_irq_enabled_with_callback(RE1_A_CLK, GPIO_IRQ_EDGE_RISE , true, &gpio_rn52_callback);
   gpio_set_irq_enabled_with_callback(RE1_B_DT,  GPIO_IRQ_EDGE_RISE , true, &gpio_rn52_callback);
   gpio_set_irq_enabled_with_callback(RE1_SW_SW, GPIO_IRQ_EDGE_RISE , true, &gpio_rn52_callback);
   gpio_set_irq_enabled_with_callback(RE2_A_CLK, GPIO_IRQ_EDGE_RISE , true, &gpio_rn52_callback);
   gpio_set_irq_enabled_with_callback(RE2_B_DT,  GPIO_IRQ_EDGE_RISE , true, &gpio_rn52_callback);
   gpio_set_irq_enabled_with_callback(RE2_SW_SW, GPIO_IRQ_EDGE_RISE , true, &gpio_rn52_callback);
}

void bt_send(uint8_t * msg, uint8_t len)
{
//   uint8_t index = 0x00;
//   while(index < len)
//   {
//      if (uart_is_writable(UART_ID)) 
//      {
//         // Change it slightly first!
//         printf("Write: %c\n",msg[index]);
//         uart_putc(UART_ID, msg[index]);
//         index++;
//      }
//      else
//      {
//         printf("uart not writable\n");
//      }
//   }
//   printf("Finished send\n");
   uart_write_blocking (UART_ID, msg, len);
}


// RX interrupt handler
void on_uart_rx(void) 
{
   uint8_t ch = uart_getc(UART_ID);
   printf("%c", ch);

   while (uart_is_readable(UART_ID))
   {
      ch = uart_getc(UART_ID);
      printf("%c", ch);

//      printf("Received: %c\n", ch);

      // Can we send it back?
//      if (uart_is_writable(UART_ID)) 
//      {
//         // Change it slightly first!
//         ch++;
//         uart_putc(UART_ID, ch);
//      }
//      chars_rxed++;
   }
}

void bt_processEvents(void)
{
   if(0x01 == bt_event_volup) {
      bt_send((uint8_t*) allRN52_cmd[RN52_CMD_VOLUP].msgPtr, allRN52_cmd[RN52_CMD_VOLUP].msgSize);
      bt_event_volup = 0x00;
   }

   if(0x01 == bt_event_voldwn) {
      bt_send((uint8_t*) allRN52_cmd[RN52_CMD_VOLDWN].msgPtr, allRN52_cmd[RN52_CMD_VOLDWN].msgSize);
      bt_event_voldwn = 0x00;
   }
   
   if(0x01 == bt_event_next) {
      bt_send((uint8_t*) allRN52_cmd[RN52_CMD_NXT].msgPtr, allRN52_cmd[RN52_CMD_NXT].msgSize);
      bt_event_next = 0x00;
   }
   
   if(0x01 == bt_event_prev) {
      bt_send((uint8_t*) allRN52_cmd[RN52_CMD_PRV].msgPtr, allRN52_cmd[RN52_CMD_PRV].msgSize);
      bt_event_prev = 0x00;
   }
   
   if(0x01 == bt_event_plps) {
      bt_send((uint8_t*) allRN52_cmd[RN52_CMD_PLP].msgPtr, allRN52_cmd[RN52_CMD_PLP].msgSize);
      bt_event_plps = 0x00;
   }
}

