/**
 * @file    re_application.c
 * @author  Jeremie Gallee (galleej@gmail.com)
 * @brief
 * @version 0.1
 * @date    2023-07-10
 *
 * @copyright Copyright (c) 2023
 *
 * @todo    add timeout for Rotary Encoders
 *
 */

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
// #include "hardware_irq.h"
#include "re_application.h"

#define RE_TIMEOUT_CLICKS_MS 100

static re_appli_handle rotary1;
static re_appli_handle rotary2;

/* RE timeout between clicks. If fired, cancel actual rotation calculation */
static alarm_id_t currentAlarm;

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
   uint32_t data = (uint32_t)(user_data);
   printf("Timer %d fired! Data: %ld\n", (int)id, data);

   /* Reset both states as we don't care about accuracy. Turn both RE at the same time is rare */
   rotary1.reState = RE_STATE_IDLE;
   rotary2.reState = RE_STATE_IDLE;
   return 0;
}

void irqhandlerRE1A(void)
{
   re_application_StateMachine(&rotary1, RE_STATE_A_KEYED);
}

void irqhandlerRE1B(void)
{
   re_application_StateMachine(&rotary1, RE_STATE_B_KEYED);
}

void irqhandlerRE1SW(void)
{
   rotary1.tokenPush = true;
}

void irqhandlerRE2A(void)
{
   re_application_StateMachine(&rotary2, RE_STATE_A_KEYED);
}

void irqhandlerRE2B(void)
{
   re_application_StateMachine(&rotary2, RE_STATE_B_KEYED);
}

void irqhandlerRE2SW(void)
{
   rotary2.tokenPush = true;
}

void re_application_StateMachine(re_appli_handle *handle, RE_STATE event)
{
   printf("RE 0x%02x - ", handle);

   switch (handle->reState)
   {
   case RE_STATE_A_KEYED:
   {
      if (RE_STATE_B_KEYED == event)
      {
         handle->tokenIndirect = true;
         printf("turn indirect\n");
      }
      /* Reset in IDLE state anyway, even if any other event.
       * Because if it is key_a again, it is maybe a glitch.
       * It means we have to re-turn again, and one step will be lost. */
      handle->reState = RE_STATE_IDLE;
      break;
   }
   case RE_STATE_B_KEYED:
   {
      if (RE_STATE_A_KEYED == event)
      {
         handle->tokenDirect = true;
         printf("turn direct\n");
      }
      /* Reset in IDLE state anyway, even if any other event.
       * Because if it is key_a again, it is maybe a glitch.
       * It means we have to re-turn again, and one step will be lost. */
      handle->reState = RE_STATE_IDLE;
      break;
   }
   case RE_STATE_IDLE:
   {
      if (RE_STATE_A_KEYED == event)
      {
         handle->reState = RE_STATE_A_KEYED;
         printf("A keyed\n");

         currentAlarm = add_alarm_in_ms(RE_TIMEOUT_CLICKS_MS, alarm_callback, NULL, false);
      }

      if (RE_STATE_B_KEYED == event)
      {
         handle->reState = RE_STATE_B_KEYED;
         printf("B keyed\n");

         currentAlarm = add_alarm_in_ms(RE_TIMEOUT_CLICKS_MS, alarm_callback, NULL, false);
      }
      break;
   }

   default:
      printf("[RE][API]Wrong RE state\n");
      break;
   }
}

void re_getHandles(uint32_t *ptrToHandle1, uint32_t *ptrToHandle2)
{
   *ptrToHandle1 = &rotary1;
   *ptrToHandle2 = &rotary2;
}

void re_initModule(void)
{
   /* Init rotary 1 */
   rotary1.reState = RE_STATE_IDLE;
   rotary1.tokenIndirect = false; /* If indirect turn of RE found */
   rotary1.tokenDirect = false;   /* If direct turn of RE found */
   rotary1.tokenPush = false;     /* If push of RE found */

   /* Init rotary 2 */
   rotary2.reState = RE_STATE_IDLE;
   rotary2.tokenIndirect = false; /* If indirect turn of RE found */
   rotary2.tokenDirect = false;   /* If direct turn of RE found */
   rotary2.tokenPush = false;     /* If push of RE found */

   printf("[RE][API] Init RE done\n");
}