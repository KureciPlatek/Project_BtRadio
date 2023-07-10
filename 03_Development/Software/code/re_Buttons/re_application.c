/**
 * @file    re_application.c
 * @author  Jeremie Gallee (@gmail.com)
 * @brief 
 * @version 0.1
 * @date    2023-07-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include "re_application.h"

static re_appli_handle rotary1;
static re_appli_handle rotary2;

void re_application_StateMachine(re_appli_handle * handle, RE_STATE event)
{
    switch (handle->reState)
    {
        case RE_STATE_A_KEYED:
        {
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
}