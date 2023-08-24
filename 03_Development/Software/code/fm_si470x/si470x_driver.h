/**
 * @file    si470x_driver.h
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief   si470x driver
 * It is an Application Programming Interface for Si4702 / Si4703 FM radio chips.
 * This file gathers all access and control of Si470x's internal registers. It
 * defines/declares a set of functions to be used, to retrieve data from Si470x
 * Register access to Si470x is a little bit strange as it is a "read until" 
 * which starts at register 0x0A.
 * This register start is not stupid, as registers 0x0A to 0x0F gathers tune and
 * RDS fetch control/data.
 * See Datasheet of Si470x of Skyworks Solutions, Inc for more details
 * 
 * Reference:
 * - Si4702/03-C19 - Broadcast FM Radio Tuner for Portable Applications (Rev. 1.1 7/09)
 * - AN230 - Si4700/01/02/03 Programming Guide (Rev. 0.9 6/09)
 * 
 * @version 0.1
 * @date    2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _SI470X_DRIVER_H_
#define _SI470X_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"
#include "si470x_driver_regs.h"
#include "rdsDecoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sensitivity settings used during seek.
 * See AN230: Seek Settings Recommendations.
 */
typedef enum {
   FM_SEEK_SENSITIVITY_DEFAULT = 0,    /* Compatible with Firmware 14 */
   FM_SEEK_SENSITIVITY_RECOMMENDED,    /* Relative to Firmware 14 */
   FM_SEEK_SENSITIVITY_MORE,           /*  Reduced SEEKTH identifies valid stations in low RSSI environments */
   FM_SEEK_SENSITIVITY_GOOD_QUALITY,   /* Identifies only good quality stations */
   FM_SEEK_SENSITIVITY_MOST,           /*  Seek algorithm relies on AFC rail */
   /* KEEP AT THE END */            
   FM_SEEK_SENSITIVITY_MAX
} FM_SEEK_SENSITIVITY;

/**
 * @brief State of FM radio module
 * State machine to allow or refuse commands done on the API
 */
typedef enum {
   SI470X_STATE_POWERED_DOWN,
   SI470X_STATE_POWERED_UP,
   SI470X_STATE_INITIALIZED,
   SI470X_STATE_CONFIGURED
} SI470X_STATE;

/**
 * @brief Frequency range in MHz corresponding to an FM_BAND.
 */
typedef struct {
   float bottom;
   float top;
} fm_freqRange_t;

/**
 * @brief Physical configuration of FM module. 
 * Values of the configuration should be adapted depending on the region
 */
typedef struct {
   FM_SEEK_SENSITIVITY  seek_sensitivity;
   FM_BAND              band;
   FM_CHANNEL_SPACING   channel_spacing;
   FM_DEEMPHASIS        deemphasis;
   FM_SOFTMUTE_RATE     softmute_rate;
   FM_SOFTMUTE_ATTEN    softmute_attenuation;
} fm_config_t;

/**
 * @brief Sensibility presets configuration
 * Si470x AN230 provides some presets about those 3 values that cmbined, have 
 * different kind of FM wave sensibility.
 * May be changed depending on FM signal quality
 */
typedef struct {
   uint8_t seekth;
   uint8_t sksnr;
   uint8_t skcnt;
} seekSens_presets_t;

/**
 * @brief Si470x handle description. It gathers all needed info, config
 * and shadowed registers
 */
typedef struct {
   SI470X_STATE   _state;     /* State of module, works like a state machine */
   fm_config_t       _config;       /* Si470x FM configuration */
   float             _freq;         /* actual frequency of radio */
   bool              _mute;         /* output audio: mute radio sound (off) */
   bool              _softmute;     /* activate soft mute or not (SNR) */
   bool              _mono;         /* avtivate mono or stereo, better SNR if in mono */
   bool              _volext;       /* output audio: extend volume range */
   uint8_t           _volume;       /* output audio: volume level */
   uint16_t          _regs[SI470x_REG_MAX];
} si470x_t;

/* ___ Region configurations and sensitivity presets description ____ */
static const seekSens_presets_t seekSensPresets[FM_SEEK_SENSITIVITY_MAX];
static const fm_freqRange_t bandRegions[FM_BAND_MAX];
static const float spacingRegions[FM_CHANNEL_SPACING_MAX];

/* ___ GETTERS ___ */
uint8_t si470x_getVolume(void);
bool si470x_getVolext(void);
bool si470x_getMono(void);
FM_SOFTMUTE_ATTEN si470x_getSoftmuteAttenuation(void);
FM_SOFTMUTE_RATE si470x_getSoftmuteRate(void);
bool si470x_getSoftmute(void);
bool si470x_getMute(void);
bool si470x_isPoweredUp(void);
fm_config_t si470x_getConfig(void);
FM_SEEK_SENSITIVITY si470x_getSeekSensitivity(void);
float si470x_getFrequency(void);

/* ______________ Start Si4703 FM module  ______________ */

/**
 * @brief Initialize the radio state and calls start-up sequence to power-up 
 * Si470x device.
 */
void si470x_init(void);

/**
 * @brief Power up the radio chip.
 * 
 * @return true   if pwer up success
 * @return false  if not
 */
bool si470x_powerUp(void);

/**
 * @brief Configure FM radio physical values and others
 * elements like region presets and so on.
 * Values are written on Si470x registers.
 */
void si470x_configureModule(void);

/**
 * @brief Power down the radio chip.
 * Puts the chip in a low power state while maintaining register configuration.
 */
void si470x_powerDown(void);

/* ______________ Trivial setters  ______________ */

/**
 * @brief set a specific sensitivity preset
 *
 * @param seek_sensitivity    FM_SEEK_SENSITIVITY, the sensitivity preset to activate
 */
void si470x_setSeekSensitivity(FM_SEEK_SENSITIVITY seek_sensitivity);

/**
 * @brief set next sensitivity preset, amongst all possible sensitivity presets in FM_SEEK_SENSITIVITY 
 */
void si470x_setSeekSensitivityNext(void);

/**
 * @brief toggle mute of output volume.
 */
void si470x_toggleMute(void);

/**
 * @brief toggle softmure of Si470x
 * Softmute is a feature in case of bad signal
 */
void si470x_toggleSoftmute(void);

/**
 * @brief Set a specific softmure level
 *
 * @param softmute_rate FM_SOFTMUTE_RATE, a preset of softmute known by Si470x
 */
void si470x_setSoftmuteRate(FM_SOFTMUTE_RATE softmute_rate);

/**
 * @brief Set a specific softmure attenuation
 * 
 * @param softmute_attenuation 
 */
void si470x_setSoftmuteAttenuation(FM_SOFTMUTE_ATTEN softmute_attenuation);

/**
 * @brief Set mono or stereo FM decoding. Les SNR with stereo
 * 
 * @param mono bool, true is mono, false stereo
 */
void si470x_setMonoStereo(bool mono);

/**
 * @brief Set volume of Si470x you may also activate volume extension
 * 
 * @param volume  uint8_t up to 15 levels
 * @param volext  bool, activate volume extension or not
 */
void si470x_setVolume(uint8_t volume, bool volext);

/**
 * @brief return STC bit status. Must be called before starting a seek or tune
 * 
 * @return true   STC bit was cleared, allowed to start a new seek or tune
 * @return false  STC bit still not cleared, SEEK or TUNE should not be started
 */
bool si470x_getSTCbit(void);

/* _______________ GPIO2 callback after long task finished  _______________ */

/**
 * @brief Call this function when SEEK or TUNE is finished, it will save 
 * inside shadow reg, on which Channel Si470x is fixed and will return 
 * the RSSI value.
 * 
 * @return uint8_t 
 */
uint8_t si470x_seekTune_finished(bool seekTune);

/**
 * @brief Get actual RDS blocks
 * 
 * @param ptrToBlocks 
 */
void si470x_getBlocks(rds_groupBlocks* ptrToBlocks);

/* ______________ Long tasks, may be blocking or asynchronous  ______________ */
/**
 * @brief set a specific frequency. 
 * May be either blocking: will poll Si470x 0Ah buffer until STC bit is set
 * Or may not be blocking, set SEEK_TUNE state machine to BUSY, until GPIO2 interrupt occurs
 * @param frequency  float, frequency to set in MHz
 * @param blocking   true: polling, false: with GPIO2 interrupt
 */
void si470x_tuneFrequency(float frequency);

/**
 * @brief Start a seek, up or down
 * 
 * @param direction 
 * @param blocking 
 */
bool si470x_startSeek(uint8_t direction);

#ifdef __cplusplus
}
#endif


/* ________ For debug _______ */
/**
 * @brief Print register values as uint8_t values with their address
 */
void readRegss2(void);

#endif // _SI470X_DRIVER_H_
