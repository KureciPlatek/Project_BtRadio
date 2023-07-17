/**
 * @file    si470x_regs.h
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief   Gathers all Si470x registers description, identifiers and masks.
 *          It also implements enumerates which translates register encoded values into
 *          human readable values/states.
 * @version 0.1
 * @date    2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _SI470X_REGS_H_
#define _SI470X_REGS_H_

#include "pico/stdlib.h"

// I2C address
#define SI4703_ADDR 0x10

/* Some Si470x limits */
#define SI4703_REGISTERS_WRITABLE   14   /* Only registers 0x02 to 0x0F are read/writable */
#define SI4703_BYTES_FOR_STD_WRITE  28   /* Only registers 0x02 to 0x0F are read/writable */
#define SI4703_BYTES_FOR_STD_READ   32   /* Consider reading all bytes */
#define SI4703_MAX_VOLUME           15 /* 4 bits register for volume */

typedef enum {
   /* Read only registers */
   SI470x_REG_DEVICEID     = 0x00,
   SI470x_REG_CHIPID       = 0x01,
   /* Read/Write registers */
   SI470x_REG_POWERCFG     = 0x02,
   SI470x_REG_CHANNEL      = 0x03,
   SI470x_REG_SYSCONFIG1   = 0x04,
   SI470x_REG_SYSCONFIG2   = 0x05,
   SI470x_REG_SYSCONFIG3   = 0x06,
   SI470x_REG_TEST1        = 0x07,
   SI470x_REG_TEST2        = 0x08,
   SI470x_REG_BOOTCONFIG   = 0x09,
   /* Start whe buffer read */
   SI470x_REG_STATUSRSSI   = 0x0A,
   SI470x_REG_READCHAN     = 0x0B,
   SI470x_REG_RDSA         = 0x0C,
   SI470x_REG_RDSB         = 0x0D,
   SI470x_REG_RDSC         = 0x0E,
   SI470x_REG_RDSD         = 0x0F,
   /* KEEP AT THE END - Automatic enum size */
   SI470x_REG_MAX
}SI470x_REGS;

/* Each register of Si470x is encoded in 16 bits. To retreive each bit or bits, 
 * use of masks.
 * Way to read and write with masks:
 *
 * _registerWrite |= SI470X_MASK_XXX & (REG_VAL << SI470X_POS_XXX);
 * _registerRead   = (_registerFromSi470x & SI470X_MASK_XXX) >> SI470X_POS_XXX;
 *
 * REG_VAL may be anything, as long as its binary value fits inside mask
 * All position in registers are in decimal
 */
 
/* Register 0x00 -   DEVICEID 
 * Manufacturer ID                        MFGID    [11:00] 
 * Part Number                            PN       [12:15] 
 */
#define SI470X_MASK_MFGID     0x0FFF
#define SI470X_POS_MFGID      0U
#define SI470X_MASK_PN_LSB    0xF000
#define SI470X_POS_PN_LSB     12U

/* Register 0x01 -   CHIPID
 * Firmware version                       FIRMWARE [05:00]
 * Device                                 DEV      [06:09]
 * Chip revision                          REV      [10:15]
 */
#define SI470X_MASK_FIRMWARE  0x003F
#define SI470X_POS_FIRMWARE   0U
#define SI470X_MASK_DEV       0x03C0
#define SI470X_POS_DEV        6U
#define SI470X_MASK_REV       0xFC00
#define SI470X_POS_REV        10U

/* Register 0x02 -   POWERCFG
 * Powerup Enable                         ENABLE   [00] 
 * reserved bits                          -        [05:01]
 * Powerup Disable                        DISABLE  [06]
 * reserved bit                           -        [07]
 * seek new channel                       SEEK     [08]
 * Seek Direction                         SEEKUP   [09]
 * Seek Mode                              SEEKMODE [10]
 * RDS Mode                               RDSM     [11]
 * reserved bit                           -        [12]
 * Mono Select                            MONO     [13]
 * Mute Disable                           DMUTE    [14]
 * Softmute Disable                       DSMUTE   [15]
 */
#define SI470X_MASK_ENABLE    0x0001
#define SI470X_POS_ENABLE     0U
#define SI470X_MASK_DISABLE   0x0040
#define SI470X_POS_DISABLE    6U
#define SI470X_MASK_SEEK      0x0100
#define SI470X_POS_SEEK       8U
#define SI470X_MASK_SEEKUP    0x0200
#define SI470X_POS_SEEKUP     9U
#define SI470X_MASK_SKMODE    0x0400
#define SI470X_POS_SKMODE     10U
#define SI470X_MASK_RDSM      0x0800
#define SI470X_POS_RDSM       11U
#define SI470X_MASK_MONO      0x2000
#define SI470X_POS_MONO       13U
#define SI470X_MASK_DMUTE     0x4000
#define SI470X_POS_DMUTE      14U
#define SI470X_MASK_DSMUTE    0x8000
#define SI470X_POS_DSMUTE     15U

/* Register 0x03 -   CHANNEL
 * Channel Select                         CHAN     [09:00]
 * reserved bits                          -        [14:10]
 * Start tune op                          TUNE     [15]
 */
#define SI470X_MASK_CHAN      0x03FF
#define SI470X_POS_CHAN       0U
#define SI470X_MASK_TUNE      0x8000
#define SI470X_POS_TUNE       15U

/* Register 0x04 -   SYSCONFIG1 
 * GPIO 1 state                           GPIO1    [01:00]
 * GPIO 2 state                           GPIO2    [03:02]
 * GPIO 3 state                           GPIO3    [05:04]
 * Stereo/Mono Blend Level Adjustment     BLNDADJ  [06:07]
 * reserved bits                          -        [09:08]
 * AGC Disable                            AGCD     [10]
 * De-emphasis                            DE       [11]
 * RDS Enable                             RDS      [12]
 * reserved bit                           -        [13]
 * Seek/Tune Complete Interrupt Enable    STCIEN   [14]
 * RDS Interrupt Enable                   RDSIEN   [15]
 */
#define SI470X_MASK_GPIO1     0x0003
#define SI470X_POS_GPIO1      0U
#define SI470X_MASK_GPIO2     0x000C
#define SI470X_POS_GPIO2      2U
#define SI470X_MASK_GPIO3     0x0030
#define SI470X_POS_GPIO3      4U
#define SI470X_MASK_BLNDADJ   0x00C0
#define SI470X_POS_BLNDADJ    7U
#define SI470X_MASK_AGCD      0x0200
#define SI470X_POS_AGCD       10U
#define SI470X_MASK_DE        0x0400
#define SI470X_POS_DE         11U
#define SI470X_MASK_RDS       0x0800
#define SI470X_POS_RDS        12U
#define SI470X_MASK_STCIEN    0x4000
#define SI470X_POS_STCIEN     14U
#define SI470X_MASK_RDSIEN    0x8000
#define SI470X_POS_RDSIEN     15U

/* Register 0x05 -   SYSCONFIG2
 * Volume                                 VOLUME   [03:00]
 * Channel Spacing                        SPACE    [05:04]
 * Band Select                            BAND     [07:06]
 * RSSI Seek Threshold                    SEEKTH   [15:08]
 */
#define SI470X_MASK_VOLUME    0x000F
#define SI470X_POS_VOLUME     0U
#define SI470X_MASK_SPACE     0x0030
#define SI470X_POS_SPACE      4U
#define SI470X_MASK_BAND      0x00C0
#define SI470X_POS_BAND       6U
#define SI470X_MASK_SEEKTH    0xFF00
#define SI470X_POS_SEEKTH     8U

/* Register 0x06 -   SYSCONFIG3
 * Seek FM Impulse Detection Threshold    SKCNT    [03:00]
 * Seek SNR Threshold                     SKSNR    [07:04]
 * Extended Volume Range                  VOLEXT   [08]
 * reserved bits                          -        [11:09]
 * Softmute Attenuation                   SMUTEA   [13:12]
 * Softmute Attack/Recover Rate           SMUTER   [15:14]
 */
#define SI470X_MASK_SKCNT     0x000F
#define SI470X_POS_SKCNT      0U
#define SI470X_MASK_SKSNR     0x00F0
#define SI470X_POS_SKSNR      4U
#define SI470X_MASK_VOLEXT    0x0100
#define SI470X_POS_VOLEXT     8U
#define SI470X_MASK_SMUTEA    0x3000
#define SI470X_POS_SMUTEA     12U
#define SI470X_MASK_SMUTER    0xF000
#define SI470X_POS_SMUTER     14U

/* Register 0x07 -   TEST1
 * reserved bits                          -        [13:00]
 * Audio High-Z Enable                    AHIZEN   [14]
 * Crystal Oscillator Enable              XOSCEN   [15]
 */
#define SI470X_MASK_AHIZEN    0x4000
#define SI470X_POS_AHIZEN     14U
#define SI470X_MASK_XOSCEN    0x8000
#define SI470X_POS_XOSCEN     15U

/* register 0x08 -   TEST2       - DO NOT USE, reserved bits only */

/* Register 0x09 -   BOOTCONFIG  - DO NOT USE, reserved bits only */ 

/* Register 0x0A -   STATUSRSSI
 * Received Signal Strength Indicator     RSSI     [07:00]
 * Stereo Indicator                       ST       [08]
 * RDS Block A Errors                     BLERA    [10:09]
 * RDS Synchronized                       RDSS     [11]
 * AFC Rail                               AFCRL    [12]
 * Seek Fail/Band Limit                   SF/BL    [13]
 * Seek/Tune Complete                     STC      [14]
 * RDS Ready                              RDSR     [15]
 */
#define SI470X_MASK_RSSI      0x00FF
#define SI470X_POS_RSSI       0U
#define SI470X_MASK_ST        0x0100
#define SI470X_POS_ST         8U
#define SI470X_MASK_BLERA     0x0600
#define SI470X_POS_BLERA      9U
#define SI470X_MASK_RDSS      0x0800
#define SI470X_POS_RDSS       11U
#define SI470X_MASK_AFCRL     0x1000
#define SI470X_POS_AFCRL      12U
#define SI470X_MASK_SFBL      0x2000
#define SI470X_POS_SFBL       13U
#define SI470X_MASK_STC       0x4000
#define SI470X_POS_STC        14U
#define SI470X_MASK_RDSR      0x8000
#define SI470X_POS_RDSR       15U

/* Register 0x0B -   READCHAN
 * Read Channel                           READCHAN [09:00]
 * RDS Block D Errors                     BLERD    [11:10]
 * RDS Block C Errors                     BLERC    [13:12]
 * RDS Block B Errors                     BLERB    [15:14]
 */
#define SI470X_MASK_READCHAN  0x03FF
#define SI470X_POS_READCHAN   0U
#define SI470X_MASK_BLERD     0x0C00
#define SI470X_POS_BLERD      10U
#define SI470X_MASK_BLERC     0x3000
#define SI470X_POS_BLERC      12U
#define SI470X_MASK_BLERB     0xC000
#define SI470X_POS_BLERB      14U

/* register 0x0C -   RDS Block A Data     Full register is RDS/BRDS data, no need of filter
 * register 0x0D -   RDS Block B Data     Full register is RDS/BRDS data, no need of filter
 * register 0x0E -   RDS Block C Data     Full register is RDS/BRDS data, no need of filter
 * register 0x0F -   RDS Block D Data     Full register is RDS/BRDS data, no need of filter
 */

/////////////////////////// VALUES INSIDE REGISTERS ////////////////////////////
/* All the following values are encoded registers values description. 
 * They should be used when writing or reading from/to Si470x.
 * Few of booleans values are "decoded", it may improve in future steps.
 */

/* ___ Register 00h. Device ID ______________________________________________ */
// - DC, single encoded values only

/* ___ Register 01h. Chip ID ________________________________________________ */
// - DC, single encoded values only

/* ___ Register 02h. Power Configuration ____________________________________ */
/* Direction of seek. */
typedef enum {
   FM_SEEK_DOWN   = 0x00,
   FM_SEEK_UP     = 0x01
} FM_SEEK_DIRECTION;

/* ___ Register 03h. Channel ________________________________________________ */
// Not encoded - Float values

/* ___ Register 04h. System Configuration 1 _________________________________ */

/* FM de-emphasis in µs */
typedef enum {
   FM_DEEMPHASIS_75     = 0x00, /**< For Americas, South Korea. */
   FM_DEEMPHASIS_50     = 0x01  /**< For Europe, Japan, Australia. */
} FM_DEEMPHASIS;

/* Stereo/Mono Blend Level Adjustment. */
typedef enum {
   FM_BLNDADJ_DEFAULT   = 0x00, /* 31–49 RSSI dBµV (default). */
   FM_BLNDADJ_P6DB      = 0x01, /* 37–55 RSSI dBµV (+6 dB).   */
   FM_BLNDADJ_M12DB     = 0x02, /* 19–37 RSSI dBµV (–12 dB)   */
   FM_BLNDADJ_M6DB      = 0x03  /* 25–43 RSSI dBµV (–6 dB)    */
}FM_BLNDADJ;

/*  General Purpose I/O 3. WARNING!!! Can't be used with Sparkfun breakboard as it is already used */
typedef enum {
   FM_GPIO3_HIGHZ       = 0x00,
   FM_GPIO3_ST          = 0x01,
   FM_GPIO3_LOW         = 0x02,
   FM_GPIO3_HIGH        = 0x03 
} FM_GPIO3;

/*  General Purpose I/O 2 */
typedef enum {
   FM_GPIO2_HIGHZ       = 0x00,
   FM_GPIO2_STCRDS      = 0x01,  /* STC/RDS interrupt. A logic high will be output unless an interrupt occurs */
   FM_GPIO2_LOW         = 0x02,
   FM_GPIO2_HIGH        = 0x03 
} FM_GPIO2;

/*  General Purpose I/O 1 */
typedef enum {
   FM_GPIO1_HIGHZ       = 0x00,
//   FM_GPIO1_RESERVED = 0x01,  /* reserved */
   FM_GPIO1_LOW         = 0x02,
   FM_GPIO1_HIGH        = 0x03 
} FM_GPIO1;

/* ___ Register 05h. System Configuration 2 _________________________________ */
/* RSSI Seek Threshold (SEEKTH) */
// Not encoded - Too complex to manipulate. Managed by function fm_set_seek_sensitivity_bits()

/* Band Select. */
typedef enum {
   FM_BAND_USAEUROPE       = 0x00,  /**< 87.5-108 MHz (default) */
   FM_BAND_JAPAN_WIDE      = 0x01,  /**< 76-108 MHz */
   FM_BAND_JAPAN           = 0x02,   /**< 76-90 MHz */
   /* KEEP AT THE END */
   FM_BAND_MAX
} FM_BAND;

/* How far apart FM channels are in kHz. */
typedef enum {
   FM_CHANNEL_SPACING_200  = 0x00, /**< For Americas, South Korea, Australia. */
   FM_CHANNEL_SPACING_100  = 0x01, /**< For Europe, Japan. */
   FM_CHANNEL_SPACING_50   = 0x02, /**< For Italy. */
   /* KEEP AT THE END */
   FM_CHANNEL_SPACING_MAX
} FM_CHANNEL_SPACING;

/* Volume */
// Not encoded

/* ___ Register 06h. System Configuration 3 _________________________________ */
/*  Softmute Attack/Recover Rate */
typedef enum {
   FM_SOFTMUTE_FASTEST        = 0x00,   /* Default on Si470x */
   FM_SOFTMUTE_FAST           = 0x01,
   FM_SOFTMUTE_SLOW           = 0x02,
   FM_SOFTMUTE_SLOWEST        = 0x03
} FM_SOFTMUTE_RATE;

/* Softmute Attenuation */
typedef enum {
   FM_SOFTMUTE_ATTENUATION_16 = 0x00,  /* Default on Si470x */
   FM_SOFTMUTE_ATTENUATION_14 = 0x01,
   FM_SOFTMUTE_ATTENUATION_12 = 0x02,
   FM_SOFTMUTE_ATTENUATION_10 = 0x03
} FM_SOFTMUTE_ATTEN;

/* Seek SNR Threshold (SKSNR). */
// Not encoded - Too complex to manipulate. Managed by function fm_set_seek_sensitivity_bits()

/* Seek FM Impulse Detection Threshold (SKCNT). */
// Not encoded - Too complex to manipulate. Managed by function fm_set_seek_sensitivity_bits()

#endif // _SI470X_REGS_H_
