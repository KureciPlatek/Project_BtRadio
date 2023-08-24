/**
 * @file    hal_main.h
 * @author  Jeremie Gallee (galleej@gmail.com)
 * @brief   Gather all GPIOs (maybe more in future) descriptions and IRQs
 *          It will call callbacks in case of GPIO triggered.
 * @version 0.1
 * @date 2023-08-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#ifndef HAL_MAIN_H
#define HAL_MAIN_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "si470x_application.h"
#include "bt_rn52_application.h"
#include "ep_application.h"
#include "re_application.h"

/* ------------ RE module ------------ */
#define RE1_GPIOSW   16 // GPIO 16 of RP2040
#define RE1_GPIOA    18  // GPIO 18 of RP2040
#define RE1_GPIOB    19  // GPIO 19 of RP2040

#define RE2_GPIOSW   17 // GPIO 17 of RP2040
#define RE2_GPIOA    20  // GPIO 20 of RP2040
#define RE2_GPIOB    21  // GPIO 21 of RP2040

/* ------------ FM module ------------ */
#define SI470X_COMM_PIN_GPIO2 6

/* ------------ BT module ------------ */
#define RN52_GPIO2   28 /* GPIO 28 for rn52's gpio2 notification pin */


/**
 * @brief Init all GPIOs
 */
void hal_initGPIOs(void);

/**
 * @brief Callback to call if ANY GPIO IRQ was triggered. It will call 
 *        corresponding module callback depending on which GPIO was triggered
 * 
 * @param gpio    [in] uint  which GPIO was triggered
 * @param events  [in] uint32_t what kind of event happened
 */
void hal_gpioCallback(uint gpio, uint32_t events);

void hal_deactivateFM(void);
void hal_deactivateBT(void);
void hal_activateFM(void);
void hal_activateBT(void);

#endif