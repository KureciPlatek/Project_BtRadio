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
#include "EPD_Test.h"   //Examples

/* _______________________________________________________ */
/*                   APPLICATION SOFTWARE                  */

#ifdef ACTIVATE_DEBUG_OUTPUT
static void manualInput(void);
static void printCommands(void);
#endif


typedef enum {
    RADIO_STATE_BT = 0,
    RADIO_STATE_FM,
    RADIO_STATE_IDLE,
    /* Keep at the end */
    RADIO_STATE_MAX
} RADIO_STATE;


/**
 * @brief Initialize Radio hardware, core and interfaces
 */
static void initRadioHW(void);

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
      radio_SM();
      sleep_ms(10);

   }

   return 0;
}

void radio_SM(void)
{
    /* Check Rotary Encoders*/
    re_appli_handle * re1;
    re_appli_handle * re2;
    getREs(re1, re2);

    switch (radioState)
    {
    case RADIO_STATE_BT:
        /* Rotary Encoder 1 - actions */
        if(true == re1->tokenIndirect)
        {
            sendVolUp();
            re1->tokenIndirect = false;
        }

        if(true == re1->tokenDirect)
        {
            sendVolDown();
            re1->tokenDirect = false;
        }

        if(true == re1->tokenPush)
        {
            sendPlayPause();
            re1->tokenPush = false;
        }

        /* Rotary Encoder 2 - actions */
        if(true == re2->tokenIndirect)
        {
            sendNextTrack();
            re2->tokenIndirect = false;
        }

        if(true == re2->tokenDirect)
        {
            sendPreviousTrack();
            re2->tokenDirect = false;
        }

        if(true == re2->tokenPush)
        {
            sendPlayPause();
            re2->tokenPush = false;
        }
        break;

    case RADIO_STATE_FM:
        /* Process inputs */
        fm_si470x_SM();

        /* Rotary Encoder 1 - actions */
        if(true == re1->tokenIndirect)
        {
            IRQ_setVolume(true);
            re1->tokenIndirect = false;
        }

        if(true == re1->tokenDirect)
        {
            IRQ_setVolume(false);
            re1->tokenDirect = false;
        }

        if(true == re1->tokenPush)
        {
            fmApp_toggleMute();
            re1->tokenPush = false;
        }

        /* Rotary Encoder 2 - actions */
        if(true == re2->tokenIndirect)
        {
            /* If we are not currently seeking */
            if(fm_get_STCbit())
            {
                /* Seek up */
                fm_seek_start(0x01);
                fmState = FM_STATE_SEEKING;
            }

            re2->tokenIndirect = false;
        }

        if(true == re2->tokenDirect)
        {
            /* If we are not currently seeking */
            if(fm_get_STCbit())
            {
                /* Seek down */
                   fm_seek_start(0x00);
                   fmState = FM_STATE_SEEKING;
                   printf("Set state to SEEK\n");
            }
            re2->tokenDirect = false;
        }

        if(true == re2->tokenPush)
        {
            /* @todo Implement big state machine of frequency/station selecting */
        }
        break;
    
    default:
        break;
    }

}


void initRadioHW(void)
{
    radioState = RADIO_STATE_IDLE;

    /* Look at GPIOs which state is active */
}



#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

void irqhandler1();
void irqhandler2();
void core1();

int main() {
    gpio_init(0);
    gpio_set_dir(0,GPIO_OUT);
    gpio_init(2);
    gpio_set_dir(1,GPIO_IN);
    gpio_init(2);
    gpio_set_dir(2,GPIO_OUT);
    gpio_init(3);
    gpio_set_dir(3,GPIO_OUT);
    gpio_init(4);
    gpio_set_dir(4,GPIO_IN);
    gpio_init(5);
    gpio_set_dir(5,GPIO_OUT);

    multicore_launch_core1(core1);

    while (true) {
        gpio_xor_mask(0b1);
        gpio_xor_mask(0b1000);
        sleep_us(10);     
    }
    return 0;
}

void core1() {
    gpio_set_irq_enabled(1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true); //monitor pin 1 connected to pin 0
    gpio_set_irq_enabled(4, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true); //monitor pin 4 connected to pin 3
    gpio_add_raw_irq_handler(1,irqhandler1);
    gpio_add_raw_irq_handler(4,irqhandler2);
    irq_set_enabled(IO_IRQ_BANK0, true);
}

//timed at 580ns to trigger
void irqhandler1() {
    if (gpio_get_irq_event_mask(1) & GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL) {
        gpio_acknowledge_irq(1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
        gpio_xor_mask(0b100);
    }
    if (gpio_get_irq_event_mask(4) & GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL) {
        gpio_acknowledge_irq(4, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
        gpio_xor_mask(0b100000);
    }
}

// duplicate as a test
void irqhandler2() {
    if (gpio_get_irq_event_mask(1) & GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL) {
        gpio_acknowledge_irq(1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
        gpio_xor_mask(0b100);
    }
    if (gpio_get_irq_event_mask(4) & GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL) {
        gpio_acknowledge_irq(4, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
        gpio_xor_mask(0b100000);
    }
}