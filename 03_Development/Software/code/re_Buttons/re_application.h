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
void re_initModule(void);

/**
 * @brief Return pointers to static handles
 *
 * @param [out] uint32_t pointer to rotary encoder 1
 * @param [out] uint32_t pointer to rotary encoder 2
 */
void re_getHandles(uint32_t *ptrToHandle1, uint32_t *ptrToHandle2);

void irqhandlerRE1A(void);
void irqhandlerRE1B(void);
void irqhandlerRE1SW(void);
void irqhandlerRE2A(void);
void irqhandlerRE2B(void);
void irqhandlerRE2SW(void);


#endif /* RE_APPLICATION_H */