/**
 * @file    re_application.h
 * @author  Jeremie Gallee (galleej@gmail.com)
 * @brief   Defines and controls rotary encoders with switch options.
 * @version 0.1
 * @date    2023-07-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdint.h>

#ifndef RE_APPLICATION_H
#define RE_APPLICATION_H

#define RE1_GPIOA 18  // GPIO 18 of RP2040
#define RE1_GPIOB 19  // GPIO 19 of RP2040
#define RE1_GPIOSW 16 // GPIO 16 of RP2040

#define RE2_GPIOA 20  // GPIO 20 of RP2040
#define RE2_GPIOB 21  // GPIO 21 of RP2040
#define RE2_GPIOSW 17 // GPIO 17 of RP2040

typedef enum
{
   RE_STATE_IDLE = 0,
   RE_STATE_A_KEYED,
   RE_STATE_B_KEYED,
   /* Keep at the end */
   RE_STATE_MAX
} RE_STATE;

/**
 * @brief Rotary Encoder handle. Instanciate one per rotary encoder
 *
 */
typedef struct
{
   /* data */
   uint32_t gpioA;     /* GPIO connected to entry B */
   uint32_t gpioB;     /* GPIO connected to entry A */
   uint32_t gpioSW;    /* GPIO connected to switch */
   RE_STATE reState;   /* State of this RE handle */
   bool tokenIndirect; /* If indirect turn of RE found */
   bool tokenDirect;   /* If direct turn of RE found */
   bool tokenPush;     /* If push of RE found */
} re_appli_handle;

/**
 * @brief   State machine of Rotary Encoders. Execute it with the RE handle when one of the GPIO becomes an interrupt
 * @warning Call it with event RE_STATE_IDLE when temout of 50-100 ms strikes. It will reset state to IDLE if we have an A or B active state
 * @param   handle of the RE on which event stroke
 */
void re_application_StateMachine(re_appli_handle *handle, RE_STATE event);

/**
 * @brief Initialize the two rotary encoders for BtRadio project
 */
void initRadioRotaries(void);

/**
 * @brief Return pointers to static handles
 *
 * @param [out] ptrToHandle1 pointer to rotary encoder 1
 * @param [out] ptrToHandle2 pointer to rotary encoder 2
 */
void getREs(re_appli_handle *ptrToHandle1, re_appli_handle *ptrToHandle2);

#endif /* RE_APPLICATION_H */