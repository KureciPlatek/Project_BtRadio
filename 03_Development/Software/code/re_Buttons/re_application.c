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

#include "hardware/gpio.h"
//#include "hardware_irq.h"
#include "re_application.h"

static re_appli_handle rotary1;
static re_appli_handle rotary2;

static void irqhandlerRE1A(void)
{
    if (gpio_get_irq_event_mask(rotary1.gpioA) & GPIO_IRQ_EDGE_RISE)
    {
        re_application_StateMachine(&rotary1, RE_STATE_A_KEYED);
    }
}

static void irqhandlerRE1B(void)
{
    if (gpio_get_irq_event_mask(rotary1.gpioB) & GPIO_IRQ_EDGE_RISE)
    {
        re_application_StateMachine(&rotary1, RE_STATE_B_KEYED);
    }
}

static void irqhandlerRE1SW(void)
{
    if (gpio_get_irq_event_mask(rotary1.gpioSW) & GPIO_IRQ_EDGE_RISE)
    {
        rotary1.tokenPush = true;
    }
}


static void irqhandlerRE2A(void)
{
    if (gpio_get_irq_event_mask(rotary2.gpioA) & GPIO_IRQ_EDGE_RISE)
    {
        re_application_StateMachine(&rotary2, RE_STATE_A_KEYED);
    }
}

static void irqhandlerRE2B(void)
{
    if (gpio_get_irq_event_mask(rotary2.gpioB) & GPIO_IRQ_EDGE_RISE)
    {
        re_application_StateMachine(&rotary2, RE_STATE_B_KEYED);
    }
}

static void irqhandlerRE2SW(void)
{
    if (gpio_get_irq_event_mask(rotary2.gpioSW) & GPIO_IRQ_EDGE_RISE)
    {
        rotary2.tokenPush = true;
    }
}

//static void alarm_irq(void) 
//{
//   // Clear the alarm irq
//   hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);
//
//   // Assume alarm 0 has fired
//   printf("Alarm IRQ fired\n");
//   state_RE1 = STATE_RE_IDLE;
//}
//
//static void alarm_in_us(uint32_t delay_us) 
//{
//   hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
//   irq_set_exclusive_handler(ALARM_IRQ, alarm_irq);
//   irq_set_enabled(ALARM_IRQ, true);
//   uint64_t target = timer_hw->timerawl + delay_us;
//   timer_hw->alarm[ALARM_NUM] = (uint32_t) target;
//}
//
//alarm_in_us(150000);    // Start delay before considering B event as void.



void re_application_StateMachine(re_appli_handle * handle, RE_STATE event)
{
    switch (handle->reState)
    {
        case RE_STATE_A_KEYED:
        {
            printf("A keyed\n");
            if(RE_STATE_B_KEYED == event)
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
            if(RE_STATE_A_KEYED == event)
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
            if(RE_STATE_A_KEYED == event)
            {
                handle->reState = RE_STATE_A_KEYED;
            }

            if(RE_STATE_B_KEYED == event)
            {
                handle->reState = RE_STATE_B_KEYED;
            }
            break;
        }

        default:
            break;
    }
}

void getREs(re_appli_handle * ptrToHandle1, re_appli_handle * ptrToHandle2)
{
    ptrToHandle1 = &rotary1;
    ptrToHandle2 = &rotary2;
}


void initRadioRotaries(void)
{
    /* Init rotary 1 */
    rotary1.gpioA  = RE1_GPIOA;
    rotary1.gpioB  = RE1_GPIOB;
    rotary1.gpioSW = RE1_GPIOSW;
    rotary1.reState = RE_STATE_IDLE;
    rotary1.tokenIndirect = false;     /* If indirect turn of RE found */
    rotary1.tokenDirect = false;       /* If direct turn of RE found */
    rotary1.tokenPush = false;         /* If push of RE found */

    /* Init rotary 2 */
    rotary2.gpioA  = RE2_GPIOA;
    rotary2.gpioB  = RE2_GPIOB;
    rotary2.gpioSW = RE2_GPIOSW;
    rotary2.reState = RE_STATE_IDLE;
    rotary2.tokenIndirect = false;     /* If indirect turn of RE found */
    rotary2.tokenDirect = false;       /* If direct turn of RE found */
    rotary2.tokenPush = false;         /* If push of RE found */

    gpio_set_irq_enabled(rotary1.gpioA, GPIO_IRQ_EDGE_RISE, true); //monitor pin 1 connected to pin 0
    gpio_set_irq_enabled(rotary1.gpioB, GPIO_IRQ_EDGE_RISE,true); //monitor pin 4 connected to pin 3
    gpio_set_irq_enabled(rotary1.gpioSW, GPIO_IRQ_EDGE_RISE, true); //monitor pin 4 connected to pin 3
    gpio_add_raw_irq_handler(rotary1.gpioA, irqhandlerRE1A);
    gpio_add_raw_irq_handler(rotary1.gpioB, irqhandlerRE1B);
    gpio_add_raw_irq_handler(rotary1.gpioSW, irqhandlerRE1SW);

    gpio_set_irq_enabled(rotary2.gpioA, GPIO_IRQ_EDGE_RISE, true); //monitor pin 1 connected to pin 0
    gpio_set_irq_enabled(rotary2.gpioB, GPIO_IRQ_EDGE_RISE,true); //monitor pin 4 connected to pin 3
    gpio_set_irq_enabled(rotary2.gpioSW, GPIO_IRQ_EDGE_RISE, true); //monitor pin 4 connected to pin 3
    gpio_add_raw_irq_handler(rotary2.gpioA, irqhandlerRE2A);
    gpio_add_raw_irq_handler(rotary2.gpioB, irqhandlerRE2B);
    gpio_add_raw_irq_handler(rotary2.gpioSW, irqhandlerRE2SW);

    irq_set_enabled(get_core_num() ? IO_IRQ_BANK1 : IO_IRQ_BANK0, true);
    printf("Core: %d\n", get_core_num());
}