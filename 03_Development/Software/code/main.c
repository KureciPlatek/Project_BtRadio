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

#include "hardware/i2c.h"
#include "EPD_Test.h"   //Examples

/* _______________________________________________________ */
/*                   APPLICATION SOFTWARE                  */

#ifdef ACTIVATE_DEBUG_OUTPUT
static void manualInput(void);
static void printCommands(void);
#endif


/**
 * @brief main function
 */
int main(void)
{
   stdio_init_all();
   
   DEV_Delay_ms(500); 

   /* Init FM module */
   printf("################## Start Prog ##################\n");
   
   /* FM module init */
//   si470x_application_init();

   /* Bluetooth module init */
   bt_init();
   
   EPD_5in83_V2_test();
   // EPD_5in83b_V2_test();

   /* Set LED ON */
   const uint LED_PIN = 25;
   gpio_init(LED_PIN);
   gpio_set_dir(LED_PIN, GPIO_OUT);
   gpio_put(LED_PIN, 1);

   while (true)
   {
      mainSM();
//      sleep_ms(10);

   }

   return 0;
}

