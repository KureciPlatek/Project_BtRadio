/**
 * @file    main.c
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief
 * @version 0.1
 * @date 2  023-03-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "fm_si470x/si470x_application.h"
#include "bt_rn52/bt_rn52_application.h"
#include <pico/stdlib.h>
#include <stdio.h>

/* Project includes */
#include "re_application.h"

#include "hardware/i2c.h"
#include "EPD_Test.h" //Examples

/* _______________________________________________________ */
/*                   APPLICATION SOFTWARE                  */

#ifdef ACTIVATE_DEBUG_OUTPUT
static void manualInput(void);
static void printCommands(void);
#endif

typedef enum
{
   RADIO_STATE_BT = 0,
   RADIO_STATE_FM,
   RADIO_STATE_IDLE,
   /* Keep at the end */
   RADIO_STATE_MAX
} RADIO_STATE;

/**
 * @brief Initialize Radio hardware, core and interfaces
 */
static void radio_init(void);

/**
 * @brief Main radio state machine
 *
 */
void radio_SM(void);

RADIO_STATE radioState;

/**
 * @brief main function
 */
int main(void)
{
   stdio_init_all();

   DEV_Delay_ms(500);

   /* Init FM module */
   printf("################## Start Prog ##################\n");
   radio_init();

   while (true)
   {
      radio_SM();
      sleep_ms(10);
   }

   return 0;
}

void radio_SM(void)
{
   /* Check Rotary Encoders*/
   re_appli_handle *re1;
   re_appli_handle *re2;
   getREs(re1, re2);

   switch (radioState)
   {
   case RADIO_STATE_BT:
      /* Rotary Encoder 1 - actions */
      if (true == re1->tokenIndirect)
      {
         bt_sendCommand(RN52_CMD_VOLUP);
         re1->tokenIndirect = false;
      }

      if (true == re1->tokenDirect)
      {
         bt_sendCommand(RN52_CMD_VOLDWN);
         re1->tokenDirect = false;
      }

      if (true == re1->tokenPush)
      {
         bt_sendCommand(RN52_CMD_PLP);
         re1->tokenPush = false;
      }

      /* Rotary Encoder 2 - actions */
      if (true == re2->tokenIndirect)
      {
         bt_sendCommand(RN52_CMD_NXT);
         re2->tokenIndirect = false;
      }

      if (true == re2->tokenDirect)
      {
         bt_sendCommand(RN52_CMD_PRV);
         re2->tokenDirect = false;
      }

      if (true == re2->tokenPush)
      {
         bt_sendCommand(RN52_CMD_PLP);
         re2->tokenPush = false;
      }
      break;

   case RADIO_STATE_FM:
      /* Process inputs */
      fm_stateMachine();

      /* Rotary Encoder 1 - actions */
      if (true == re1->tokenIndirect)
      {
         fm_setVolume(true);
         re1->tokenIndirect = false;
      }

      if (true == re1->tokenDirect)
      {
         fm_setVolume(false);
         re1->tokenDirect = false;
      }

      if (true == re1->tokenPush)
      {
         fm_toggleMute();
         re1->tokenPush = false;
      }

      /* Rotary Encoder 2 - actions */
      if (true == re2->tokenIndirect)
      {
         /* If we are not currently seeking */
         if (fm_get_STCbit())
         {
            /* Seek up */
            fm_startSeek(0x01);
            fm_setState(FM_STATE_SEEKING);
         }
         re2->tokenIndirect = false;
      }

      if (true == re2->tokenDirect)
      {
         /* If we are not currently seeking */
         if (fm_get_STCbit())
         {
            /* Seek down */
            fm_startSeek(0x00);
            fm_setState(FM_STATE_SEEKING);
         }
         re2->tokenDirect = false;
      }

      if (true == re2->tokenPush)
      {
         /* @todo Implement big state machine of frequency/station selecting */
      }
      break;

   default:
      break;
   }
}

void radio_init(void)
{
   radioState = RADIO_STATE_IDLE;

   /* FM module init */
   //   fm_init();

   /* Bluetooth module init */
   bt_init();

   EPD_5in83_V2_test();
   // EPD_5in83b_V2_test();

   /* Set LED ON */
   const uint LED_PIN = 25;
   gpio_init(LED_PIN);
   gpio_set_dir(LED_PIN, GPIO_OUT);
   gpio_put(LED_PIN, 1);

   /* Look at GPIOs which state is active */
}
