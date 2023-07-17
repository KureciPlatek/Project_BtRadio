/**
 * @file    si470x_api.c
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief   API to use Si470x module from upper application layers. 
 *          Use it to seek/tune FM radio stations, get RDS data and more.
 *          It needs some RP2040 SDK modules/libraries
 * @version 0.1
 * @date    2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "si470x_comm.h"
#include "si470x_api.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/**
 * @TODO add following features:
 *
 * ______________________________________
 * - Chip ID, rev and concstructor check: verify it has RDS and is good
 * - get RSSI bits (STATUSRSSI register) -> Furnishes RSSI level 
 * (nice to see, poll it for smooth pulse on HMI)
 * - RDS/RBDS supported  -> Check for API config & state machine
 * - AGCD Automatic Gain Control -> Actually, better keep it on
 * - ST stereo indicator (ST bit) -> maybe do it, but we have mono output, what for? 
 * - BLNDADJ Stereo goes with with -> feature to be checked
 *
 * ______________________________________
 * SHOULD BE CHECKED:
 * - RDSM: RDS mode (verbose or not)
 * - RDSIEN: RDS Interrupt ENable  (on GPIO2)
 * - STCIEN: Seek Tune Complete Interrupt ENable (on GPIO2)
 * 
 * ______________________________________
 * UNCHECKED that could be good to add:
 * - AFCRL AFC Rail (automatic frequency control) 1 = AFC railed, indicating an invalid channel
 * Using the RSSI threshold in conjunction with AFC rail offers seek performance 
 * with a greater than 90% probability of finding only valid stations
 * - AHIZEN (audio high Z enable) for electronic output protection purpose? To be checked
 * - SF/BL
 * - RDSR   RDS group Ready
 * - RDSS   RDS synchronized. Available only in RDS Verbose mode
 * 
 * ______________________________________
 * rework way to configure region, and the different Si470x chips: some have
 * RDS/RBDS, some not.
 * 
 * 
 * @WARNING, check status with restart seek/tune when flag is not resetted in Si470x memory
 * ANSWER -> Do not set the TUNE or SEEK bits until the Si470x clears the STC bit.
 */

// change this to configure FM band, channel spacing, and de-emphasis
#define RADIO_ID_STR_SIZE 5

#define SI4703_REG_SIZE_BYTES       32
#define SI4703_REG_RDS_SIZE_BYTES   8

#define TUNE_POLL_INTERVAL_MS 20U
#define SEEK_POLL_INTERVAL_MS 200U // relatively large, to reduce electrical interference from I2C

/* ___ Region configurations and sensitivity presets description ____ */
/* SEEKTH, SKSNR, SKCNT */
static const seekSens_presets_t seekSensPresets[] =
{{0x19, 0x00, 0x00}, /* FM_SEEK_SENSITIVITY_DEFAULT */
 {0x19, 0x04, 0x08}, /* FM_SEEK_SENSITIVITY_RECOMMENDED */
 {0x0C, 0x04, 0x08}, /* FM_SEEK_SENSITIVITY_MORE */
 {0x0C, 0x07, 0x0F}, /* FM_SEEK_SENSITIVITY_GOOD_QUALITY */
 {0x00, 0x04, 0x0F}  /* FM_SEEK_SENSITIVITY_MOST */
};

/* BOTTOM , TOP */
static const fm_freqRange_t bandRegions[] = 
{{87.5f, 108.0f}, /* FM_BAND_USAEUROPE */
 {76.0f, 108.0f}, /* FM_BAND_JAPAN_WIDE */
 {76.0f, 90.0f }    /* FM_BAND_JAPAN */
};

/* FREQ SPACE */
static const float spacingRegions[] = 
{0.2f,   /* FM_CHANNEL_SPACING_200 Americas, South Korea, Australia. */
 0.1f,   /* FM_CHANNEL_SPACING_100  Europe, Japan. */
 0.05f,  /* FM_CHANNEL_SPACING_50   Italy. */
};

/* Handle to Si470x module, gathers all needed info and registers */
si470x_t _radioHandle;

void readRegss2(void)
{
   uint8_t tempRegs[SI4703_BYTES_FOR_STD_READ];
   uint8_t index = 0;
   int writeRes = 0;

   /* Send to I2C - nonstop set to false, otherwise, error in Si470x internal register addr counter */
   writeRes = i2c_read_blocking(i2c_default, SI4703_ADDR, tempRegs, SI4703_BYTES_FOR_STD_READ, false);
   if(writeRes == SI4703_BYTES_FOR_STD_READ)
   {
      for(;index <SI4703_BYTES_FOR_STD_READ; index++)
      {
         if(0 == (index % 2)) {printf("\nh%02x", ((index/2+0xA)%0x10));}
         printf("[0x%02X]", tempRegs[index]);
      }
      printf("\n");
   }
   else
   {
      printf("[ERROR] I2C - read regs\n");
   }
}


///////////////////// PUBLIC FUNCTIONS DEFINITIONS /////////////////////////////
void si470x_init(void) 
{
   printf("[FM][API] init fm module\n");
   uint8_t index = 0x00;
   uint16_t regVal = 0x0000;
   
   /* Init hardware communication line for Si470x module */
   fm_initCommHW();

   /* reset memory where _radioHandle is placed to 0 */
   memset(&_radioHandle, 0, sizeof(si470x_t));

   /* Set EUROPE configuration */
   _radioHandle._config.band                 = FM_BAND_USAEUROPE;
   _radioHandle._config.channel_spacing      = FM_CHANNEL_SPACING_100;
   _radioHandle._config.deemphasis           = FM_DEEMPHASIS_50;
   /* Set default FM softmute technic and sensitivity */
   _radioHandle._config.seek_sensitivity     = FM_SEEK_SENSITIVITY_RECOMMENDED;
   _radioHandle._config.softmute_rate        = FM_SOFTMUTE_FASTEST;
   _radioHandle._config.softmute_attenuation = FM_SOFTMUTE_ATTENUATION_16;

   /* Set default frequency with region presets as bottom spectrum freq */
   _radioHandle._freq      = bandRegions[_radioHandle._config.band].bottom;
   _radioHandle._mute      = false;
   _radioHandle._softmute  = true;
   _radioHandle._mono      = true;  /* start in mono, better SNR */
   _radioHandle._volext    = false; /* High volume range */
   _radioHandle._volume    = SI4703_MAX_VOLUME;
   
   /* All Si470x registers values (shadow) should be 0 */
   while((index < SI470x_REG_MAX) &&( 0x0000 == regVal))
   {
      regVal = _radioHandle._regs[index];
      if(0x0000 != regVal) {printf("[FM][API] ERROR - one shadow reg not 0x0000 (Reg%d=0x%04x\n", index, regVal);}
      index++;
   };
   
   /* Power up FM module, to power it down, call fm_deInit() or fm_power_down() */
   _radioHandle._state = SI470X_STATE_INITIALIZED;
//   fm_power_up();
   printf("[FM][API] INIT - radio init finished\n");
}

bool fm_power_up(void) 
{
   printf("[FM][API] INIT - radio power up\n");
   bool retVal = true;
   uint16_t readReg[SI470x_REG_BOOTCONFIG];
   uint8_t  DEV_bits   = 0x00;
   uint8_t  index      = 0x00;

   /* ____ START OF SEQUENCE - Bus Configuration - see AN230 _______ */
   /* Seems that PICO I2C SDIO must be pulled low for 2 wires op. */
   gpio_put(PICO_DEFAULT_I2C_SDA_PIN, 0);
   /* Put reset pin anyway to LOW */
   fm_si470x_setRST(PIN_LOW);
   
   /* As I2C is initiated, SDIO should be pulled high (mandatory) for method 1 init */
   fm_si470x_setSEN(PIN_HIGH);

   // End Bus Mode Selection
   fm_si470x_setRST(PIN_HIGH);
   sleep_ms(5);
   /* ____ END OF SEQUENCE. From here registers may be read/write ____  */

   /* Check registers */
   /* Save only config registers. RSSI, READCHAN and RDS regs not needed*/
   if (!fm_si470x_readRegisters(&readReg[0], SI470x_REG_BOOTCONFIG)) 
   {
      printf("[FM][API] ERROR - I2C read failure, check wirings\r\n");
      retVal = false;
   }
   else
   {
      /* Save all registers into shadow register on RP2040 */
      for(index = 0; index < SI470x_REG_BOOTCONFIG; index++)
      {
         _radioHandle._regs[index] = readReg[index];
//         printf("REG_%d = [0x%04x]\n", index, _radioHandle._regs[index]);
      }

      /* __ Configure Si470x hardware __ */
      _radioHandle._regs[SI470x_REG_TEST1]    |= SI470X_MASK_XOSCEN	& ( 1 << SI470X_POS_XOSCEN);   /* 1= use internal oscillator, GPIO3 config ignored */
      _radioHandle._regs[SI470x_REG_RDSD]      = 0x0000; // Si4703-C19 errata - ensure RDSD register is zero
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_RDSD);
      sleep_ms(600); // wait for oscillator to stabilize

      /* Finish power up sequence by setting the following registers */
      _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_ENABLE	& (1 << SI470X_POS_ENABLE);   
      _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_DISABLE	& (0 << SI470X_POS_DISABLE);   
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_POWERCFG);
      sleep_ms(100); // wait for device powerup
   
      /* Check device powered up */
      if(fm_si470x_readRegisters(&readReg[0], SI470x_REG_BOOTCONFIG))
      {
         DEV_bits   = (readReg[SI470x_REG_CHIPID]   & SI470X_MASK_DEV)    >> SI470X_POS_DEV;

         if(0x3C04 != (readReg[SI470x_REG_TEST1] & 0x3FFF))
         {
            printf("TEST1 reg value error: 0x%04x\n", readReg[SI470x_REG_TEST1]);
         }
         
         /* @TODO set kind of Si470x device it is (some don't have RDS decoding) */
         if(0x09 == DEV_bits)
         {
            _radioHandle._state = SI470X_STATE_POWERED_UP;
            /* Next sequence, configure radio */
            printf("[FM][API] INIT - Radio power up OK (0x%02x)\n", DEV_bits);
            fm_configure_radio();
         }
         else
         {
            printf("[FM][API] ERROR - with power up: %d\n", DEV_bits);
            retVal = false;
         }
      }
   }
   
   return retVal;
}

void fm_configure_radio(void)
{
   printf("[FM][API] INIT - radio configure\n");

   uint16_t readReg[SI470x_REG_BOOTCONFIG];
   uint8_t index = 0;
   uint16_t test1TempReg = 0x0000;

   for(index = 0; index < SI470x_REG_BOOTCONFIG; index++)
   {
      readReg[index] = 0x0000;
   }

   /* Error while doing the following stuff, TEST1 reg is overwritten */
   if(fm_si470x_readRegisters(&readReg[0], SI470x_REG_BOOTCONFIG))
   {
      test1TempReg = readReg[SI470x_REG_TEST1];
//      printf("Saved TEST1: 0x%04x\n", test1TempReg);
   }
   else
   {
      printf("Could not save TEST1 register\n");
   }

   /* Start radio output and listening */
   _radioHandle._regs[SI470x_REG_POWERCFG]   |= SI470X_MASK_MONO     & (_radioHandle._mono << SI470X_POS_MONO);
   _radioHandle._regs[SI470x_REG_POWERCFG]   |= SI470X_MASK_DMUTE    & (_radioHandle._mute << SI470X_POS_DMUTE);
   
   /* Activate GPIO2 as interrupt - XOSCEN ERRATA: better use GPIO2 interrupt instead of polling if using internal oscillator */
   _radioHandle._regs[SI470x_REG_SYSCONFIG1] |= SI470X_MASK_RDSIEN   & (1 << SI470X_POS_RDSIEN);
   _radioHandle._regs[SI470x_REG_SYSCONFIG1] |= SI470X_MASK_STCIEN   & (1 << SI470X_POS_STCIEN);
   _radioHandle._regs[SI470x_REG_SYSCONFIG1] |= SI470X_MASK_GPIO2    & (0x01 << SI470X_POS_GPIO2); 

   _radioHandle._regs[SI470x_REG_SYSCONFIG1] |= SI470X_MASK_RDS      & (1 << SI470X_POS_RDS);
   _radioHandle._regs[SI470x_REG_SYSCONFIG1] |= SI470X_MASK_DE	      & ( _radioHandle._config.deemphasis << SI470X_POS_DE);   
   _radioHandle._regs[SI470x_REG_SYSCONFIG2] |= SI470X_MASK_BAND     & ( _radioHandle._config.band << SI470X_POS_BAND);
   _radioHandle._regs[SI470x_REG_SYSCONFIG2] |= SI470X_MASK_SPACE	   & ( _radioHandle._config.channel_spacing << SI470X_POS_SPACE);
   _radioHandle._regs[SI470x_REG_SYSCONFIG2] |= SI470X_MASK_VOLUME   & ( 0x6 << SI470X_POS_VOLUME);
   _radioHandle._regs[SI470x_REG_SYSCONFIG2] |= SI470X_MASK_SEEKTH   & (seekSensPresets[_radioHandle._config.seek_sensitivity].seekth << SI470X_POS_SEEKTH);
   _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SKCNT    & (seekSensPresets[_radioHandle._config.seek_sensitivity].skcnt << SI470X_POS_SKCNT);
   _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SKSNR    & (seekSensPresets[_radioHandle._config.seek_sensitivity].sksnr << SI470X_POS_SKSNR);      
   _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SMUTER   & ( _radioHandle._config.softmute_rate << SI470X_POS_SMUTER);
   _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SMUTEA   & ( _radioHandle._config.softmute_attenuation << SI470X_POS_SMUTEA);
   _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_VOLEXT   & ( _radioHandle._volext << SI470X_POS_VOLEXT);

   /* Force TEST1 to its previous value */
   _radioHandle._regs[SI470x_REG_TEST1] = test1TempReg;
   
   /* Write up to config3 */
   fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_SYSCONFIG3);

   /* Check all registers correctly written */
   if (!fm_si470x_readRegisters(&readReg[0], SI470x_REG_BOOTCONFIG)) 
   {
      printf("[FM][API] ERROR - I2C read failure, check wirings\r\n");
   }
   else
   {
      printf("[FM][API] INIT - Radio configure finished\n");
      for(index = 0; index < SI470x_REG_BOOTCONFIG; index++)
      {
         _radioHandle._regs[index] = readReg[index];
//         printf("Reg_%d - [0x%04x]\n", index, readReg[index]);
      }
      if(0x3C04 != (readReg[SI470x_REG_TEST1] & 0x3FFF))
      {
         printf("TEST1 reg wrong value: 0x%04x - 0x%04x\n", readReg[SI470x_REG_TEST1], test1TempReg);
      }
      else
      {
         _radioHandle._state = SI470X_STATE_CONFIGURED;
      }
   }
}

void fm_power_down(void) 
{
   if(SI470X_STATE_POWERED_UP == _radioHandle._state)
   {
      _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SMUTER   & ( _radioHandle._config.softmute_rate << SI470X_POS_SMUTER);
      
      /* For Si4703: Recommended to disable RDS before powering down. See AN230 rev 0.9 */
      _radioHandle._regs[SI470x_REG_SYSCONFIG1] |= SI470X_MASK_RDS      & (0 << SI470X_POS_RDS);
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_SYSCONFIG1);
   
      _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_DMUTE   & (0 << SI470X_POS_DMUTE);
      _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_DISABLE & (1 << SI470X_POS_DISABLE);
   
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_POWERCFG);
      _radioHandle._state = SI470X_STATE_POWERED_DOWN;
      printf("[FM][API] Si470x powered down\n");
   }
}

void fm_tune_frequency(float frequency)
{
   printf("[FM][API] Tune freq %f - start\n", frequency);
   uint16_t tempReg;   /* We read STATUSRSSI only */
   uint8_t bitSTC = 0x01;

   if(SI470X_STATE_POWERED_UP <= _radioHandle._state)
   {
      /* Check STC cleared before new seek */
      fm_si470x_readRegisters(&tempReg, SI470x_REG_STATUSRSSI);
      bitSTC = (tempReg & SI470X_MASK_STC) >> SI470X_POS_STC;

      /* No tune if same freq or if STC not yet cleared */
      if((_radioHandle._freq != frequency) && (0x00 == bitSTC))
      {
         uint16_t channel = (uint16_t)roundf((frequency - bandRegions[_radioHandle._config.band].bottom) 
                                                     / spacingRegions[_radioHandle._config.channel_spacing]);

         /* Write computed channel to registers and start tuning */
         _radioHandle._regs[SI470x_REG_CHANNEL] |= SI470X_MASK_CHAN & (channel << SI470X_POS_CHAN);
         _radioHandle._regs[SI470x_REG_CHANNEL] |= SI470X_MASK_TUNE & (1 << SI470X_POS_TUNE);
         fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_CHANNEL);
      }
      else
      {
         printf("[FM][API] Tune failed. STC: %d, Freq: %f | %f\n", bitSTC, frequency, _radioHandle._freq);
      }
   }
   else
   {
      printf("[FM][API] ERROR - No tune possible, Si470x powered down: %d\n", _radioHandle._state);
   }
   printf("Tune on going...\n");
}

void fm_startSeek(uint8_t direction)
{
   printf("[FM][API] Seek start\n");
   uint16_t tempRegs[0x02];   /* We read registers STATUSRSSI and READCHANNEL */
   uint8_t bitSTC = 0x01;

   /* Fool proof */
   if((0x00 == direction) || (0x01 == direction)) 
   {
      /* Check STC cleared before new seek */
      fm_si470x_readRegisters(&tempRegs[0], SI470x_REG_READCHAN);
      bitSTC = (tempRegs[0] & SI470X_MASK_STC) >> SI470X_POS_STC;

      if(0x00 == bitSTC)
      {
         /* WRAP MODE - Bit SF/BL will be set to 1 if no channel good enough found */
         _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_SKMODE & (0 << SI470X_POS_SKMODE);
         /* Set POWERCFG_SEEKUP to high to seek high and to low to seek low */
         _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_SEEKUP & (direction << SI470X_POS_SEEKUP);
         _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_SEEK   & (1 << SI470X_POS_SEEK);
         /* Write registers and start seek */
         printf(" --- Start seek\n");
         if(fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_POWERCFG))
         {
            printf("Send seek cmd ok\n");
         }
      }
      else
      {
         printf("[FM][API] - STC bit not yet cleared\n");
      }
   }
   printf("z");
}

uint8_t fm_seekTune_finished(bool seekTune)
{
   printf("[FM][API] Seek tune finished\n");
   uint16_t tempRegs[SI470x_REG_READCHAN+1];   

   uint8_t SFBL_bit  = 0x00;
//   uint8_t ST_bit    = 0x00;
   uint8_t RSSI_bits = 0x00;
   uint16_t channel  = 0x0000;

   /* Seek Tune finished, save CHANNEL and return RSSI */
   if(fm_si470x_readRegisters(&tempRegs[0], SI470x_REG_READCHAN))
   {
//      ST_bit    = (tempRegs[SI470x_REG_STATUSRSSI] & SI470X_MASK_ST)   >> SI470X_POS_ST;
      RSSI_bits = (tempRegs[SI470x_REG_STATUSRSSI] & SI470X_MASK_RSSI) >> SI470X_POS_RSSI;
      channel   = (tempRegs[SI470x_REG_READCHAN]   & SI470X_MASK_READCHAN) >> SI470X_POS_READCHAN;

      /* If action was SEEK */
      if(seekTune)
      {
         SFBL_bit  = (tempRegs[SI470x_REG_STATUSRSSI] & SI470X_MASK_SFBL) >> SI470X_POS_SFBL;
         /* End SEEK */
         _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_SEEK   & (0 << SI470X_POS_SEEK);
         if(!fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_POWERCFG))
         {
            printf("Error while clearing bit SEEK\n");
         }

         if(0x01 != SFBL_bit)
         {
            _radioHandle._freq = channel * spacingRegions[_radioHandle._config.channel_spacing] 
                                         + bandRegions[_radioHandle._config.band].bottom;
         }
         else
         {
            /* Return RSSI = 0 if failure while seek (return an error) */
            RSSI_bits = 0x00;
         }
      }
      /* If action was TUNE */
      else
      {
         _radioHandle._freq = channel * spacingRegions[_radioHandle._config.channel_spacing] 
                                         + bandRegions[_radioHandle._config.band].bottom;

         /* End TUNE */
         _radioHandle._regs[SI470x_REG_CHANNEL] |= SI470X_MASK_TUNE   & (0 << SI470X_POS_TUNE);
         if(!fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_CHANNEL))
         {
            printf("Error while clearing bit TUNE\n");
         }
      }
   }
   else
   {
      printf("SEEK/TUNE finished - error while reading registers\n");
   }
   return RSSI_bits;
}

void si470x_toggleMute(void) 
{
   if(_radioHandle._state != SI470X_STATE_POWERED_DOWN) 
   {
      if (_radioHandle._mute) 
      {
         _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_DMUTE & (0 << SI470X_POS_DMUTE);
         _radioHandle._mute = false;
      }
      else
      {
         _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_DMUTE & (1 << SI470X_POS_DMUTE);
         _radioHandle._mute = true;
      }

      printf("[FM][API] Mute unmute\n");
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_POWERCFG);
   }
}

void fm_toggle_softmute(void) 
{
   if(_radioHandle._state != SI470X_STATE_POWERED_DOWN) 
   {
      if (_radioHandle._softmute) 
      {
         _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_DSMUTE & (0 << SI470X_POS_DSMUTE);
         _radioHandle._softmute = false;
      }
      else
      {
         _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_DSMUTE & (1 << SI470X_POS_DSMUTE);
         _radioHandle._softmute = true;
      }

      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_POWERCFG);
   }
}

void fm_set_seek_sensitivity(FM_SEEK_SENSITIVITY seek_sensitivity) 
{
   if ((_radioHandle._state != SI470X_STATE_POWERED_DOWN) 
    && (_radioHandle._config.seek_sensitivity != seek_sensitivity))
   {
      /* Set corresponding registers for wished sensibility preset (see AN230) */
      _radioHandle._regs[SI470x_REG_SYSCONFIG2] |= SI470X_MASK_SEEKTH & (seekSensPresets[seek_sensitivity].seekth << SI470X_POS_SEEKTH);
      _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SKCNT  & (seekSensPresets[seek_sensitivity].skcnt  << SI470X_POS_SKCNT);
      _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SKSNR  & (seekSensPresets[seek_sensitivity].sksnr  << SI470X_POS_SKSNR);      
      /* Write down to FM module */
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_SYSCONFIG3);
      /* Update shadow registers */
      _radioHandle._config.seek_sensitivity = seek_sensitivity;
   }
}

void fm_set_seek_sensitivity_next(void)
{
   FM_SEEK_SENSITIVITY seek_sensitivity;

   if (_radioHandle._state != SI470X_STATE_POWERED_DOWN) 
   {
      /* Go to next sensibility */
      seek_sensitivity = (_radioHandle._config.seek_sensitivity + 1) % FM_SEEK_SENSITIVITY_MAX;
      /* Set corresponding registers for wished sensibility preset (see AN230) */
      _radioHandle._regs[SI470x_REG_SYSCONFIG2] |= SI470X_MASK_SEEKTH & (seekSensPresets[seek_sensitivity].seekth << SI470X_POS_SEEKTH);
      _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SKCNT  & (seekSensPresets[seek_sensitivity].skcnt << SI470X_POS_SKCNT);
      _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SKSNR  & (seekSensPresets[seek_sensitivity].sksnr << SI470X_POS_SKSNR);      

      /* Write down to FM module */
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_SYSCONFIG3);
      /* Update shadow registers */
      _radioHandle._config.seek_sensitivity = seek_sensitivity;
   }
}

void fm_set_softmute_rate(FM_SOFTMUTE_RATE softmute_rate) 
{
   if( (_radioHandle._state != SI470X_STATE_POWERED_DOWN) 
    && (_radioHandle._config.softmute_rate != softmute_rate))
   {
      _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SMUTER & (softmute_rate << SI470X_POS_SMUTER);
      _radioHandle._config.softmute_rate = softmute_rate;
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_SYSCONFIG3);
   }
}

void fm_set_softmute_attenuation(FM_SOFTMUTE_ATTEN softmute_attenuation) 
{
   if( (_radioHandle._state != SI470X_STATE_POWERED_DOWN) 
    && (_radioHandle._config.softmute_attenuation != softmute_attenuation))
   {
      _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_SMUTEA & (softmute_attenuation << SI470X_POS_SMUTEA);
      _radioHandle._config.softmute_attenuation = softmute_attenuation;

      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_SYSCONFIG3);
   }
}

void fm_set_monoStereo(bool mono) 
{
   if( (_radioHandle._state != SI470X_STATE_POWERED_DOWN) 
    && (_radioHandle._mono != mono))
   {
      _radioHandle._regs[SI470x_REG_POWERCFG] |= SI470X_MASK_MONO & (mono << SI470X_POS_MONO);
      _radioHandle._mono = mono;

      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_SYSCONFIG3);
   }
}

void si470x_set_volume(uint8_t volume, bool volext) 
{
   if(_radioHandle._state  != SI470X_STATE_POWERED_DOWN) 
   {
      /* Avoid volume higher than Si470x max volume */
      volume = MIN(volume, SI4703_MAX_VOLUME);

      _radioHandle._regs[SI470x_REG_SYSCONFIG2] |= SI470X_MASK_VOLUME & (volume << SI470X_POS_VOLUME);
      _radioHandle._regs[SI470x_REG_SYSCONFIG3] |= SI470X_MASK_VOLEXT & (volext << SI470X_POS_VOLEXT);
      _radioHandle._volume = volume;
      _radioHandle._volext = volext;
 
      printf("set volume to %d\n", volume);
      fm_si470x_writeRegisters(&_radioHandle._regs[0], SI470x_REG_SYSCONFIG3);
   }
   printf("PROUT\n");
}

void fm_getBlocks(rds_groupBlocks* ptrToBlocks)
{
   uint16_t upperRegs[0x06]; /* Only upper registers 0Ah to 0Fh */

   if(fm_si470x_readRegisters(&upperRegs[0], SI470x_REG_RDSD))
   {
      /* Check if RDS ready*/
      if(0x01 == (upperRegs[0] & SI470X_MASK_RDSR) >> SI470X_POS_RDSR)
      {
         /* Two first registers 0Ah and 0Bh don´t have RDS data */
         ptrToBlocks->block_A = upperRegs[2];
         ptrToBlocks->block_B = upperRegs[3];
         ptrToBlocks->block_C = upperRegs[4];
         ptrToBlocks->block_D = upperRegs[5];
      }
      else
      {
         printf("RDSR not ready: %d", ((upperRegs[0] & SI470X_MASK_RDSR) >> SI470X_POS_RDSR));
      }
   }
}


bool fm_get_STCbit(void)
{
   uint16_t tempRegs[SI470x_REG_STATUSRSSI+1];   
   bool retVal = false;
   uint8_t STC_bit = 0x00;

   /* Seek Tune finished, save CHANNEL and return RSSI */
   if(fm_si470x_readRegisters(&tempRegs[0], SI470x_REG_STATUSRSSI))
   {
      STC_bit = (tempRegs[SI470x_REG_STATUSRSSI] & SI470X_MASK_STC) >> SI470X_POS_STC;
      if(0x00 == STC_bit)
      {
         retVal = true;
      }
   }
   else
   {
      printf("ERROR while getting STC bit\n");
   }
   return retVal;
}


/* ___ GETTERS ___ */
uint8_t fm_get_volume(void) 
{ return _radioHandle._volume; }

bool fm_get_volext(void) 
{ return _radioHandle._volext; }

bool fm_get_mono(void) 
{ return _radioHandle._mono; }

FM_SOFTMUTE_ATTEN fm_get_softmute_attenuation(void) 
{ return _radioHandle._config.softmute_attenuation; }

FM_SOFTMUTE_RATE fm_get_softmute_rate(void)
{ return _radioHandle._config.softmute_rate; }

bool fm_get_softmute(void)
{ return _radioHandle._softmute; }

bool fm_get_mute(void)
{ return _radioHandle._mute; }

bool fm_is_powered_up(void)
{ return (_radioHandle._state == SI470X_STATE_POWERED_UP) ? true : false; }

fm_config_t fm_get_config(void)
{ return _radioHandle._config; }

FM_SEEK_SENSITIVITY fm_get_seek_sensitivity(void)
{ return _radioHandle._config.seek_sensitivity; }

float fm_get_frequency(void)
{ return _radioHandle._freq; }
