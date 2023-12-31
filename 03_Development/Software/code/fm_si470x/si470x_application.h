/**
 * @file    si470x_application.h
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief   High level file of modules. Contains API for applicative firmwares
 * @version 0.1
 * @date    2023-02-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _SI470X_APPLICATION_H_
#define _SI470X_APPLICATION_H_

#include <stdint.h>
#include "si470x_driver.h"
#include "rdsDecoder.h"

#define MAX_PRESETS 10

typedef enum{
   FM_STATE_INIT = 0,
   FM_STATE_PWRUP,
   FM_STATE_PWRDWN,
   FM_STATE_IDLE,
   FM_STATE_TUNING,
   FM_STATE_SEEKING,
   FM_STATE_RDS,
   /* Keep at the end */
   FM_STATE_MAX
} FM_STATE;

typedef enum {
   ROTARY_CLOCKWISE = 0,
   ROTARY_ANTICLOCKWISE,
   /* Keep at the end */
   ROTARY_MAX
} ROTARY;

typedef struct {
   float preset_freq;
   char  preset_PSname[PS_GROUP_MAX_CHARS+1];   /* Add +1 for \0 end of chain char */
} fm_station_preset;

/**
 * @brief All station presets freq and station name
 */
static fm_station_preset stationsPresets[MAX_PRESETS];

/**
 * @brief init Si4703 module application layer
 */
void fm_init(void);

void fm_deactivate(void);
void fm_activate(void);

/**
 * @brief Callback in case of GPIO2 interrupt from si470x
 */
void fm_si470xGpio2_callback(void);

/**
 * @brief main state machine of FM demodulator module
 */
void fm_stateMachine(void);

/**
 * @brief Set volume of Si4704 FM module
 * 
 * @param upDown true: more sound baby, false, darling is sleeping
 */
void fm_setVolume(bool upDown);

/**
 * @brief OH YEAH I LOVE THIS FM STATION! Then save it into a specific
 *        preset index. The way you provide index is up to you.
 * 
 * @param indexPresets  uint8_t, the preset index
 */
void fm_saveTuneFreq(uint8_t indexPresets);

/**
 * @brief Print saved station on stdio
 */
void fm_printStationPresets(void);

/**
 * @brief toggle Mute, direct call, avoid call of api from outside module
 */
void fm_toggleMute(void);

/**
 * @brief  Start seeking a channel. It firstly check if STC bit was
 *         cleared. Which meands that previous seek or tune was
 *         complete. And it will initate a new seek tune sequence.
 * 
 * @param  [in] upDown 0x01 to search up, 0x00 to search down 
 * @return true if start seeking worked
 * @return false if not
 */
bool fm_startSeekChannel(uint8_t upDown);

#endif /* _SI470X_APPLICATION_H_ */