/**
 * @file    hal_main.c
 * @author  Jeremie Gallee (galleej@gmail.com)
 * @brief   Gather all GPIOs (maybe more in future) descriptions and IRQs
 *          It will call callbacks in case of GPIO triggered.
 * @version 0.1
 * @date 2023-08-08
 * 
 * @copyright Copyright (c) 2023
 * 
 *    "LEVEL_LOW",  // 0x1
 *    "LEVEL_HIGH", // 0x2
 *    "EDGE_FALL",  // 0x4
 *    "EDGE_RISE"   // 0x8
 * 
 */

#include "hal_main.h"

void hal_initGPIOs(void)
{

   /* ------------------------------- RN52 ------------------------------- */
   gpio_set_irq_enabled_with_callback(RN52_GPIO2, GPIO_IRQ_EDGE_FALL, true, &hal_gpioCallback);

   /* ------------------------------- SI470X ------------------------------- */
   /* Declare callback of GPIO2*/
   gpio_init(SI470X_COMM_PIN_GPIO2);
   gpio_set_dir(SI470X_COMM_PIN_GPIO2, GPIO_IN);
   gpio_set_irq_enabled(SI470X_COMM_PIN_GPIO2, GPIO_IRQ_EDGE_FALL, true);

   /* ------------------------------- Rotary encoders ------------------------------- */
   gpio_set_irq_enabled(RE1_GPIOA,  GPIO_IRQ_EDGE_RISE, true);
   gpio_set_irq_enabled(RE1_GPIOB,  GPIO_IRQ_EDGE_RISE, true);
   gpio_set_irq_enabled(RE1_GPIOSW, GPIO_IRQ_EDGE_RISE, true);

   gpio_set_irq_enabled(RE2_GPIOA,  GPIO_IRQ_EDGE_RISE, true);
   gpio_set_irq_enabled(RE2_GPIOB,  GPIO_IRQ_EDGE_RISE, true);
   gpio_set_irq_enabled(RE2_GPIOSW, GPIO_IRQ_EDGE_RISE, true);
}

void hal_gpioCallback(uint gpio, uint32_t events)
{
   if ((GPIO_IRQ_EDGE_FALL == events) && (RN52_GPIO2 == gpio))
   {
      printf("[HAL][GPIO] RN52 gpio2 IRQ call\n");
      rn52_handleGpio2();
   }
   if ((GPIO_IRQ_EDGE_FALL == events) && (SI470X_COMM_PIN_GPIO2 == gpio))
   {
      fm_si470xGpio2_callback();
   }

   if(GPIO_IRQ_EDGE_RISE == events)
   {
      switch (gpio)
      {
      case RE1_GPIOA:
         irqhandlerRE1A();
         break;
      case RE1_GPIOB:
         irqhandlerRE1B();
         break;
      case RE1_GPIOSW:
         irqhandlerRE1SW();
         break;
      case RE2_GPIOA:
         irqhandlerRE2A();
         break;
      case RE2_GPIOB:
         irqhandlerRE2B();
         break;
      case RE2_GPIOSW:
         irqhandlerRE2SW();
         break;
      
      default:
         break;
      }
   }
}
