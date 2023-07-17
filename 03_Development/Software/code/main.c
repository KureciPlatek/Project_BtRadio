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
#include "EPD_Test.h" //Examples @todo remove

#define GPIO_FM_MODE 27 /* GPIO connected to upper exclusives buttons. It sets the active mode */
#define GPIO_BT_MODE 26 /* GPIO connected to upper exclusives buttons. It sets the active mode */
/* GPIO to activate or deactivate hardware module, depending on modes
 * If high, BT deactivated, FM activated
 * If low,  FM deactivated, BT activated 
 * It will also change analog audio switch to the correct position 
 */
#define GPIO_MODE_HW 22
#define LED_PIN      25 /* LED on Pico Board. It takes no further GPIO on board */


/* _______________________________________________________ */
/*                   APPLICATION SOFTWARE                  */
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
 * @brief Get state of radio by reading which button on upper side
 * is currently active 
 */
static void radio_getMode(void);

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

   /* Wait for HW to stabilize - may be measured if can be shorter */
   DEV_Delay_ms(50);

   /* Initialize radio and its modules as wished */
   radio_init();

   printf("################## Start Prog ##################\n");
   while (true)
   {
      /* Infinite loop */
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
   re_getHandles(re1, re2);

   /* Poll activated mode (gpio) */
   radio_getMode();

   /* Execute radio main state machine to process the last inputs */
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

   /* Rotary Encoders init */
   re_initModule();

   /* FM module init */
   //   fm_init();

   /* Bluetooth module init */
   bt_init();

   /* e-Paper module init */
   // EPD_5in83_V2_test();
   // EPD_5in83b_V2_test();

   /* Set Pico Board LED ON */
   const uint LED_PIN = 25;
   gpio_init(LED_PIN);
   gpio_set_dir(LED_PIN, GPIO_OUT);
   gpio_put(LED_PIN, 1);

   /* Radio get mode configuration (2 pins which are not module specific) */
   gpio_init(GPIO_BT_MODE);
   gpio_init(GPIO_FM_MODE);
   gpio_set_dir(GPIO_BT_MODE, GPIO_IN);
   gpio_set_dir(GPIO_FM_MODE, GPIO_IN);

   /* Radio output mode hardware activation. 
    It will activate or deactivate FM or BT hardware modules */
   gpio_init(GPIO_MODE_HW);
   gpio_set_dir(GPIO_MODE_HW, GPIO_OUT);
   gpio_put(GPIO_MODE_HW, 0); /* Bluetooth module per default */
}

static void radio_getMode(void)
{
   if(true == gpio_get(GPIO_BT_MODE))
   {
      ep_write(PLACE_ACTIVEMODE, "Bluetooth activated");
      radioState = RADIO_STATE_BT;
      gpio_put(GPIO_MODE_HW, 1);
   }
   /* Deactivated until si470x works */
//   else if(true == gpio_get(GPIO_FM_MODE))
//   {
//      radioState = RADIO_STATE_FM;
//      gpio_put(GPIO_MODE_HW, 0);
//   }
   else
   {
      radioState = RADIO_STATE_IDLE;
      ep_write(PLACE_ACTIVEMODE, "No mode selected (FM or Bluetooth)");
      gpio_put(GPIO_MODE_HW, 0); /* Bluetooth module per default */
   }
}