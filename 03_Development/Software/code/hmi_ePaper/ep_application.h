/**
 * @file    ep_application.h
 * @author  Jeremie Gallee (galleej@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-07-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#ifndef EP_APPLICATION_H
#define EP_APPLICATION_H

/* C standard includes */
#include <stdlib.h>

/* Project includes */

/* Module includes */
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "EPD_5in83_V2.h"

typedef enum {
   EPAPER_PLACE_BT_TRACK = 0,
   EPAPER_PLACE_BT_STATUS,
   EPAPER_PLACE_FM_FAVORITES,
   EPAPER_PLACE_ACTIVEMODE,
   /* Keep at the end */
   EPAPER_PLACE_MAX
} EPAPER_PLACE;

/**
 * @brief   initialize e-paper and its required place in memory
 * 
 * @return  true  if enough free heap to control epaper and if init of epaper HW is good
 * @return  false if not
 */
bool ep_init(void);

/**
 * @brief   Write string at a specified place on screen
 * 
 * @param   EPAPER_PLACE [in] enumerate, will write lines at this defined place
 * @param   line         [in] Starting line in screen section 
 * @param   char*        [in] pointer to string of char to print on epaper
 * @return  true  if write text success
 * @return  false if not
 */
bool ep_write(EPAPER_PLACE place, uint8_t line, char * ptrToString);

/**
 * @brief   Clear and deactivate screen
 * 
 * @return  true  if deactivation success
 * @return  false if not
 */
bool ep_deactivate(void);


#endif /* EP_APPLICATION_H */