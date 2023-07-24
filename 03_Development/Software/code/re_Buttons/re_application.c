/**
 * @file    re_application.c
 * @author  Jeremie Gallee (@gmail.com)
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

static void irqhandlerRE1A(uint gpio, uint32_t events)
{
   if ((GPIO_IRQ_EDGE_RISE == events)
    && (rotary1.gpioA == gpio))
   {
      re_application_StateMachine(&rotary1, RE_STATE_A_KEYED);
   }
}

static void irqhandlerRE1B(uint gpio, uint32_t events)
{
//   if (gpio_get_irq_event_mask(rotary1.gpioB) & GPIO_IRQ_EDGE_RISE)
   if ((GPIO_IRQ_EDGE_RISE == events)
    && (rotary1.gpioB == gpio))
   {
      re_application_StateMachine(&rotary1, RE_STATE_B_KEYED);
   }
}

static void irqhandlerRE1SW(uint gpio, uint32_t events)
{
//   if (gpio_get_irq_event_mask(rotary1.gpioSW) & GPIO_IRQ_EDGE_RISE)
   if ((GPIO_IRQ_EDGE_RISE == events)
    && (rotary1.gpioSW == gpio))
   {
      rotary1.tokenPush = true;
   }
}

static void irqhandlerRE2A(uint gpio, uint32_t events)
{
//   if (gpio_get_irq_event_mask(rotary2.gpioA) & GPIO_IRQ_EDGE_RISE)
   if ((GPIO_IRQ_EDGE_RISE == events)
    && (rotary2.gpioA == gpio))
   {
      re_application_StateMachine(&rotary2, RE_STATE_A_KEYED);
   }
}

static void irqhandlerRE2B(uint gpio, uint32_t events)
{
//   if (gpio_get_irq_event_mask(rotary2.gpioB) & GPIO_IRQ_EDGE_RISE)
   if ((GPIO_IRQ_EDGE_RISE == events)
    && (rotary2.gpioB == gpio))
   {
      re_application_StateMachine(&rotary2, RE_STATE_B_KEYED);
   }
}

static void irqhandlerRE2SW(uint gpio, uint32_t events)
{
//   if (gpio_get_irq_event_mask(rotary2.gpioSW) & GPIO_IRQ_EDGE_RISE)
   if ((GPIO_IRQ_EDGE_RISE == events)
    && (rotary2.gpioSW == gpio))
   {
      rotary2.tokenPush = true;
   }
}

void re_application_StateMachine(re_appli_handle *handle, RE_STATE event)
{
   switch (handle->reState)
   {
   case RE_STATE_A_KEYED:
   {
      printf("A keyed\n");
      if (RE_STATE_B_KEYED == event)
      {
         handle->tokenIndirect = true;
      }
      /* Reset in IDLE state anyway, even if any other event.
       * Because if it is key_a again, it is maybe a glitch.
       * It means we have to re-turn again, and one step will be lost. */
      handle->reState = RE_STATE_IDLE;
      break;
   }
   case RE_STATE_B_KEYED:
   {
      printf("B keyed\n");
      if (RE_STATE_A_KEYED == event)
      {
         handle->tokenDirect = true;
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

         /* Start timer - other pin of RE should be triggered there */
         if (false == cancel_alarm(currentAlarm))
         {
            printf("Error 4, can't deactivate actual alarm");
         }
         currentAlarm = add_alarm_in_ms(RE_TIMEOUT_CLICKS_MS, alarm_callback, NULL, false);
      }

      if (RE_STATE_B_KEYED == event)
      {
         handle->reState = RE_STATE_B_KEYED;

         /* Start timer - other pin of RE should be triggered there */
         if (false == cancel_alarm(currentAlarm))
         {
            printf("Error 3, can't deactivate actual alarm");
         }
         currentAlarm = add_alarm_in_ms(RE_TIMEOUT_CLICKS_MS, alarm_callback, NULL, false);
      }
      break;
   }

   default:
      break;
   }
}

void re_getHandles(re_appli_handle *ptrToHandle1, re_appli_handle *ptrToHandle2)
{
   ptrToHandle1 = &rotary1;
   ptrToHandle2 = &rotary2;
}

void re_initModule(void)
{
   /* Init rotary 1 */
   rotary1.gpioA = RE1_GPIOA;
   rotary1.gpioB = RE1_GPIOB;
   rotary1.gpioSW = RE1_GPIOSW;
   rotary1.reState = RE_STATE_IDLE;
   rotary1.tokenIndirect = false; /* If indirect turn of RE found */
   rotary1.tokenDirect = false;   /* If direct turn of RE found */
   rotary1.tokenPush = false;     /* If push of RE found */

   /* Init rotary 2 */
   rotary2.gpioA = RE2_GPIOA;
   rotary2.gpioB = RE2_GPIOB;
   rotary2.gpioSW = RE2_GPIOSW;
   rotary2.reState = RE_STATE_IDLE;
   rotary2.tokenIndirect = false; /* If indirect turn of RE found */
   rotary2.tokenDirect = false;   /* If direct turn of RE found */
   rotary2.tokenPush = false;     /* If push of RE found */

//   gpio_set_irq_enabled(rotary1.gpioA, GPIO_IRQ_EDGE_RISE, true);  // monitor pin 1 connected to pin 0
//   gpio_set_irq_enabled(rotary1.gpioB, GPIO_IRQ_EDGE_RISE, true);  // monitor pin 4 connected to pin 3
//   gpio_set_irq_enabled(rotary1.gpioSW, GPIO_IRQ_EDGE_RISE, true); // monitor pin 4 connected to pin 3
//   gpio_add_raw_irq_handler(rotary1.gpioA,  irqhandlerRE1A);
//   gpio_add_raw_irq_handler(rotary1.gpioB,  irqhandlerRE1B);
//   gpio_add_raw_irq_handler(rotary1.gpioSW, irqhandlerRE1SW);
   gpio_set_irq_enabled_with_callback(rotary1.gpioA,  GPIO_IRQ_EDGE_RISE, true, &irqhandlerRE1A);
   gpio_set_irq_enabled_with_callback(rotary1.gpioB,  GPIO_IRQ_EDGE_RISE, true, &irqhandlerRE1B);
   gpio_set_irq_enabled_with_callback(rotary1.gpioSW, GPIO_IRQ_EDGE_RISE, true, &irqhandlerRE1SW);


//   gpio_set_irq_enabled(rotary2.gpioA, GPIO_IRQ_EDGE_RISE, true);  // monitor pin 1 connected to pin 0
//   gpio_set_irq_enabled(rotary2.gpioB, GPIO_IRQ_EDGE_RISE, true);  // monitor pin 4 connected to pin 3
//   gpio_set_irq_enabled(rotary2.gpioSW, GPIO_IRQ_EDGE_RISE, true); // monitor pin 4 connected to pin 3
//   gpio_add_raw_irq_handler(rotary2.gpioA,  irqhandlerRE2A );
//   gpio_add_raw_irq_handler(rotary2.gpioB,  irqhandlerRE2B );
//   gpio_add_raw_irq_handler(rotary2.gpioSW, irqhandlerRE2SW);
   gpio_set_irq_enabled_with_callback(rotary2.gpioA,  GPIO_IRQ_EDGE_RISE, true, &irqhandlerRE2A);
   gpio_set_irq_enabled_with_callback(rotary2.gpioB,  GPIO_IRQ_EDGE_RISE, true, &irqhandlerRE2B);
   gpio_set_irq_enabled_with_callback(rotary2.gpioSW, GPIO_IRQ_EDGE_RISE, true, &irqhandlerRE2SW);

   /* @todo check stuff here */
//   irq_set_enabled(get_core_num() ? IO_IRQ_BANK1 : IO_IRQ_BANK0, true);
//   printf("Core: %d\n", get_core_num());
}