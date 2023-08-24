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

#define EPAPER_CHARS_PER_LINE 30

/**
 * @brief enumerate to list different places where categories
 * of info/strings may be printed
 */
typedef enum {
   EPAPER_PLACE_ACTIVEMODE = 0,
   EPAPER_PLACE_BT_STATUS,
   EPAPER_PLACE_BT_TRACK,
   EPAPER_PLACE_FM_FAVORITES,
   /* Keep at the end */
   EPAPER_PLACE_MAX
} EPAPER_PLACE;

/**
 * @brief For each place described earlier, describe on screen where it should be
 * 
 */
typedef struct {
   uint16_t coordinates_X;
   uint16_t coordinates_Y;
   sFONT * desiredFont;
} epaperConfig;

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
 * @param   flush        [in] boole if we direct write on epaper, or just write some lines and call ep_flush later
 * @return  true  if write text success
 * @return  false if not
 */
bool ep_write(EPAPER_PLACE place, uint8_t line, char * ptrToString, bool flush);

/**
 * @brief   Clear and deactivate screen
 * 
 * @return  true  if deactivation success
 * @return  false if not
 */
bool ep_deactivate(void);

/**
 * @brief Clean image buffer as it uses a huge part of heap which may be
 * polluted with old code and have remanent bits which produces stranges 
 * images.
 * Should be called only at start-up.
 */
void ep_cleanImageBUffer(void);

/**
 * @brief To avoid all time refresh, when writing couple of lines,
 * just write lines, then call this function to print all to epaper.
 */
void ep_flush(void);


#endif /* EP_APPLICATION_H */