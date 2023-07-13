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
#include "si470x_api.h"
#include "si470x_comm.h"
#include "rdsDecoder.h"
#include "bt_rn52_application.h"

static void printCommands(void);
static void manualInput(void);

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
uint8_t tokenIRQ_GPIO2 = 0x00;

static char event_str[128];
static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};
//void gpio_event_string(char *buf, uint32_t events);
//void gpio_callback(uint gpio, uint32_t events) 
//{
//    // Put the GPIO event(s) that just happened into event_str
//    // so we can print it
////    printf("GPIO2 event: %d - event: 0x%04x\n", gpio, events);
//    gpio_event_string(event_str, events);
//    printf("GPIO %d %s\n", gpio, event_str);
//    tokenIRQ_GPIO2 = 0x01;
//}
//
//void gpio_event_string(char *buf, uint32_t events) {
//    for (uint i = 0; i < 4; i++) {
//        uint mask = (1 << i);
//        if (events & mask) {
//            // Copy this event string into the user string
//            const char *event_str = gpio_irq_str[i];
//            while (*event_str != '\0') {
//                *buf++ = *event_str++;
//            }
//            events &= ~mask;
//
//            // If more events add ", "
//            if (events) {
//                *buf++ = ',';
//                *buf++ = ' ';
//            }
//        }
//    }
//    *buf++ = '\0';
//}

void fm_stateMachine(void)
{
   manualInput();

   switch(fmState)
   {
      case FM_STATE_INIT:
         /* Automatically go to power up (may be changed in future), this step should be done by fm_init() */
         fmState = FM_STATE_PWRUP; 
         break;
      case FM_STATE_PWRUP:
         /* Power up and configure radio */
         if(fm_power_up())
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
         fm_power_down();
         break;
      case FM_STATE_IDLE:
         /* Wait for Seek or tune event from human */
         break;
      case FM_STATE_SEEKING:
         if(0x01 == tokenIRQ_GPIO2)
         {
            processSTCEvent(true);
            tokenIRQ_GPIO2 = 0x00;
         }
//         printf(".");
         break;
      case FM_STATE_TUNING:
         if(0x01 == tokenIRQ_GPIO2)
         {
            processSTCEvent(false);
            tokenIRQ_GPIO2 = 0x00;
         }
//         printf("-");
         break;
      case FM_STATE_RDS:
         if(0x01 == tokenIRQ_GPIO2)
         {
            processRDSEvent();
            tokenIRQ_GPIO2 = 0x00;
         }
//         printf(";");
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
   fm_power_up();

   /* Declare callback of GPIO2*/
   gpio_init(SI470X_COMM_PIN_GPIO2);
   gpio_set_dir(SI470X_COMM_PIN_GPIO2, GPIO_IN);
   gpio_set_irq_enabled_with_callback(SI470X_COMM_PIN_GPIO2, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

   /* Init RDS decoder, less quality for more verbose */
   rdsDecoder_init(RT_STATE_QUALITY_BAD, false);

   fmState = FM_STATE_INIT;
}

void fm_setVolume(bool upDown)
{
   uint8_t fmModuleVolume = fm_get_volume();
   if((true == upDown) && (SI4703_MAX_VOLUME > fmModuleVolume))
   {
      fmModuleVolume++;
   }
   else if((false == upDown) && (0 < fmModuleVolume))
   {
      fmModuleVolume--;
   }
   /* else, already at max or min*/
   si470x_set_volume(fmModuleVolume, false); /* @TODO check volume Extension use */
}

void fm_saveTuneFreq(uint8_t indexPresets)
{
   if(MAX_PRESETS >= indexPresets)
   {
      uint8_t pointerToPSname[PS_GROUP_MAX_CHARS];
      uint8_t index = 0x00;

      stationsPresets[indexPresets].preset_freq = fm_get_frequency();

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

void fm_setState(FM_STATE state)
{
   fmState = state;
}


void processSTCEvent(bool seekTune)
{
   uint8_t data_RSSI;
   data_RSSI = fm_seekTune_finished(seekTune);

   if(0 < data_RSSI)
   {
      printf("SeekTune finished, RSSI: %d, Freq: %f", data_RSSI, fm_get_frequency());
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

void processRDSEvent(void)
{
   uint8_t rtMsg[RT_GROUP_MAX_CHARS];
   uint8_t sizeOfRTmsg = 0x00;
   rds_groupBlocks tempGroup;
   uint8_t index = 0;

   /* GPIO2 interrupt = we have RDS data */
   fm_getBlocks(&tempGroup);
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


////////////////////////////// _____ HMI _____ ////////////////////////////// 
//
///* Print commands on stdio */
//static void printCommands(void)
//{
//   printf("____ Si470x command list_____\n");
//   printf("u - seek up\n");
//   printf("k - ask for Bt track data\n");
//   printf("q - ask for Bt q\n");
//   printf("d - seek down\n");
//   printf("+ - increase volume\n");
//   printf("- - decrease volume\n");
//   printf("m - mute or unmute\n");
//   printf("p - print presets\n");
//   printf("X - enter the preset number you want to tune (0 to 9)\n");
//   printf("h - print this help\n");
//   printf("s - Try read regs and print them\n");
//   printf("_____________________________\n");
//}
//
//static void manualInput(void)
//{
//   int result = getchar_timeout_us(1);
//   if (result != PICO_ERROR_TIMEOUT)
//   {
//      char cmd = (char)result;
//      printf("New cmd: %c\n", cmd);
//
//      /* Seek or Tune allowed only in those states: */
//      if((fmState == FM_STATE_IDLE) 
//      || (fmState == FM_STATE_RDS))
//      {
//         switch (cmd)
//         {
/* -------------------------- BT module commands ------------------------ */            
//            case 'u':
//               if(fm_get_STCbit())
//               {
////                  IRQ_seekNext(ROTARY_CLOCKWISE);
//                  fm_startSeek(0x01);
//                  fmState = FM_STATE_SEEKING;
//               }
//               break;
//            case 'd':
//               if(fm_get_STCbit())
//               {
////                  IRQ_seekNext(ROTARY_ANTICLOCKWISE);
//                  fm_startSeek(0x00);
//                  fmState = FM_STATE_SEEKING;
//                  printf("Set state to SEEK\n");
//               }
//               break;
//            case '1':
//               if(fm_get_STCbit())
//               {
//                  fmState = FM_STATE_TUNING;
//                  float frequency = stationsPresets[0].preset_freq;
//                  fm_tune_frequency(frequency);
//               }
//               break;
//            case '2':
//               if(fm_get_STCbit())
//               {
//                  fmState = FM_STATE_TUNING;
//                  float frequency = stationsPresets[1].preset_freq;
//                  fm_tune_frequency(frequency);
//                  fmState = FM_STATE_TUNING;
//               }
//               break;
//            case '+':
//               {
//               fm_setVolume(true);
//               }
//               break;
//            case '-':
//               {
//               fm_setVolume(false);
//               }
//               break;
//            case 'm':
//               fm_toggleMute();
//               break;
//            case 's':
//               readRegss2();
//               break;
//            default: 
//               printf("Command disabled - FM module tuning or seeking - keep I2C quiet\n");
//               break;
//         }
//      }
//
//      /* Command that may be used anytime */
//      switch (cmd)
//      {
//         case 'p':
//            fm_printStationPresets();
//            break;
//         case 'h':
//            printCommands();
//            break;
//         default:
//            break;
//      }
//   }
//}