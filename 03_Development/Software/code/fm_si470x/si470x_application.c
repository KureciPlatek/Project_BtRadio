/**
 * @file    si470x_application.c
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief   Application/behavior file of Si470x. 
 *          This file sets the wished behavior of FM module, when we use 
 *          the follofwing functions:
 *             - Seeking
 *             - Tuning
 *             - Read RT messages
 *             - save favorite channel
 * @version 0.1
 * @date 2023-02-23
 * 
 * @copyright Copyright (c) 2023
 */

#include <stdio.h>
#include "si470x_application.h"
#include "si470x_comm.h"
#include "rdsDecoder.h"
#include "bt_rn52_application.h"
#include "ep_application.h"
#include "hal_main.h"

/* @todo add doxygen comments for those functions */
static void processSTCEvent(bool seekTune);
static void processRDSEvent(void);

static fm_station_preset stationsPresets[] =
{{88.8f,  "BernObrl\0"},  /* Radio Bern oberland, OOOOOOOOHHH YEAAAHHH */
 {88.9f,  "not set \0"},
 {90.0f,  "not set \0"},
 {91.0f,  "not set \0"},
 {92.0f,  "not set \0"},
 {93.2f,  "Clasic21\0"},  /* Classic 21 */
 {100.0f, "not set \0"},
 {104.0f, "not set \0"},
 {106.0f, "not set \0"},
 {107.7f, "Neue1077\0"}   /* THE Neue 107.7 Stuttgart love it */
};

FM_STATE fmState;
/* Notify IRQ to process the interrupt */
uint8_t tokenIRQ_GPIO2 = 0x00;

void fm_si470xGpio2_callback(void)
{
   if((FM_STATE_INIT  != fmState) 
   && (FM_STATE_PWRUP != fmState))
   {
      tokenIRQ_GPIO2 = 0x01;
//      fm_stateMachine();
   }
}

void fm_stateMachine(void)
{
   switch(fmState)
   {
      case FM_STATE_INIT:
         /* Automatically go to power up (may be changed in future), this step should be done by fm_init() */
         fmState = FM_STATE_PWRUP; 
         break;
      case FM_STATE_PWRUP:
         /* Power up and configure radio */
         if(si470x_powerUp())
         {
            /* All good, wait for human commands */
            fmState = FM_STATE_IDLE;
         }
         else
         {
            printf("[FM][APP] - FATAL while powering up Si470x module\n");
         }
         break;
      case FM_STATE_PWRDWN:
         si470x_powerDown();
         break;
      case FM_STATE_IDLE:
         /* Wait for Seek or tune event from human */
//         ep_write(EPAPER_PLACE_ACTIVEMODE, 0, "Radio - FM demodulator", true);

         break;
      case FM_STATE_SEEKING:
         if(0x01 == tokenIRQ_GPIO2)
         {
            processSTCEvent(true);
            tokenIRQ_GPIO2 = 0x00;
         }
         break;
      case FM_STATE_TUNING:
         if(0x01 == tokenIRQ_GPIO2)
         {
            processSTCEvent(false);
            tokenIRQ_GPIO2 = 0x00;
         }
         break;
      case FM_STATE_RDS:
         if(0x01 == tokenIRQ_GPIO2)
         {
            processRDSEvent();
            tokenIRQ_GPIO2 = 0x00;
         }
         break;
      case FM_STATE_MAX:
      default:
         printf("ERROR - unknown state of FM module: 0x%02x\n", fmState);
         break;
   }
}

void fm_init(void)
{
   printf("init application\n");

   /* Init Si470x module */
   si470x_init();

   si470x_powerUp();

   /* Declare callback of GPIO2 done in hal_gpio */

   /* Init RDS decoder, less quality for more verbose */
   rdsDecoder_init(RT_STATE_QUALITY_BAD, false);

   fmState = FM_STATE_IDLE;
}

void fm_deactivate(void)
{
   hal_deactivateFM();
}

void fm_activate(void)
{
   hal_activateFM();
}

void fm_setVolume(bool upDown)
{
   uint8_t fmModuleVolume = si470x_getVolume();
   if((true == upDown) && (SI4703_MAX_VOLUME > fmModuleVolume))
   {
      fmModuleVolume++;
   }
   else if((false == upDown) && (0 < fmModuleVolume))
   {
      fmModuleVolume--;
   }
   /* else, already at max or min*/
   si470x_setVolume(fmModuleVolume, false); /* @TODO check volume Extension use */
}

void fm_saveTuneFreq(uint8_t indexPresets)
{
   if(MAX_PRESETS >= indexPresets)
   {
      uint8_t pointerToPSname[PS_GROUP_MAX_CHARS];
      uint8_t index = 0x00;

      stationsPresets[indexPresets].preset_freq = si470x_getFrequency();

      /* @TODO check if PS name is good */
      rdsDecoder_getPS(&pointerToPSname[0]);

      for(;index < PS_GROUP_MAX_CHARS; index++)
      {
         stationsPresets[indexPresets].preset_PSname[index] = *(pointerToPSname + index);
      }
   }
}

void fm_printStationPresets(void)
{
   uint8_t index = 0;
   for(; index < MAX_PRESETS ;index++)
   {
      printf("Station %d: %s - FM: %f\n", (index+1), stationsPresets[index].preset_PSname, stationsPresets[index].preset_freq);
   }
}

void fm_toggleMute(void)
{
   si470x_toggleMute();
}

bool fm_startSeekChannel(uint8_t upDown)
{
   bool retVal = false;

   if (si470x_getSTCbit())
   {
      if(true == si470x_startSeek(upDown))
      {
         fmState = FM_STATE_SEEKING;
         retVal  = true;
      }
   }

   return retVal;
}

static void processSTCEvent(bool seekTune)
{
   uint8_t data_RSSI;
   data_RSSI = si470x_seekTune_finished(seekTune);

   if(0 < data_RSSI)
   {
      printf("SeekTune finished, RSSI: %d, Freq: %f", data_RSSI, si470x_getFrequency());
      fmState = FM_STATE_RDS; /* Finished with SeekTune, go to RDS decoding */
   }
   else
   {
      /* Error while SeekTune, return to IDLE state */
      /* @TODO If state was SEEKING. maybe try some other seek sensibility */
      fmState = FM_STATE_IDLE;
      printf("Seek or Tune error\n");
   }
}

static void processRDSEvent(void)
{
   uint8_t rtMsg[RT_GROUP_MAX_CHARS];
   uint8_t sizeOfRTmsg = 0x00;
   rds_groupBlocks tempGroup;
   uint8_t index = 0;

   /* GPIO2 interrupt = we have RDS data */
   si470x_getBlocks(&tempGroup);
   sizeOfRTmsg = rdsDecoder_processNewGroup(&rtMsg[0], &tempGroup);
   if(0 < sizeOfRTmsg)
   {
      index = 0;
      for(;index < sizeOfRTmsg; index++)
      {
         printf("%c", rtMsg[index]);
      }
   }
   /* @TODO mechanism to keep reading new track names. 
      It changes even if we stay on same channel. If jump to
      IDLE state, no further read of RT msg will be done */
}
