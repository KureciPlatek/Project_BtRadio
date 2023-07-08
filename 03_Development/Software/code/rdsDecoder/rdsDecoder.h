/**
 * \copyright (c) 2021 Valentin Milea <valentin.milea@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 * \file rds_parser.h
 *
 * \brief RDS data parser.
 * This is a basic parser for the most commonly transmitted RDS information.
 */

#ifndef _RDS_PARSER_H_
#define _RDS_PARSER_H_

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**______________________________________
 * RDS/RBDS decoder
 * 
 * Please read first:
 * - wikipedia page: https://en.wikipedia.org/wiki/Radio_Data_System
 * - AN243 USING RDS/RBDS WITH THE Si4701/03 from Skyworks
 * - AN230 Si4700/01/02/03 PROGRAMMING GUIDE from Skyworks
 *
 * Features and functions of RDS Parser:
 * Many functions are available with RDS/RBDS technology. But onyl few are
 * implemented in this module. They are on my opinion, irrelevant and doesn't
 * bring end-user pertinent infotainement features.
 * They may be developped and supported in future, or added by other developpers
 * 
 * Featuer wanted of a radio by an end-user:
 * - Feature 1: listen to something whil I am doing something else, chilling:
 *    IF(I like the sound I am listening)
 *       THEN Keep listening
 *    ELSE
 *       Seek next available station
 *    ENDIF
 *
 * - Feature 2: save and switch to some channels I like
 * #ifdef REGION_ENGLAND_PUB
 *    IF(I like listening this channel every morning while drinking beer)
 * #else
 *    IF(I like listening this channel every morning while drinking coffee)
 * #endif
 *       THEN - Save it
 *            - Affect a button for instant acces next morning
 *
 * I don't think that making more complex features is very useful. Radio is not 
 * a complex tool that we want to tailor "au petits oignons". Just start it,
 * listen, switch channel, enjoy.
 * 
 * ______________________________________
 * SUPPORTED FEATURES:
 * - RT (Radio Text)
 * - PS (Program Service Name)
 * 
 * ______________________________________
 * UNSUPPORTED FEATURES:
 * - EON (enhanced other networks information) -> Switch to next radio with good quality is more important than exclusively listening to one type of stuff
 * - PI (programme identification) -> like it is important to see stuff like PI=C201 (which means BBC Radio)...
 * - PTY (programme type) -> it is not because we have a program type, that we like the content...
 * - TA, TP (traffic announcement, traffic programme) -> If you are stuck in traffic jam, try biking or train
 * - TMC (traffic message channel) -> It is soooo 1990-2000. Now we use other mean which are way much better and makes this thing obsolete
 * 
 * ______________________________________
 * FUTURE DEVELOPMENTS:
 * - AF (alternative frequencies)
 * - CT (clock time and date)
 * - RT+ (radio text plus)
 * - RT messages errors correction
 * 
 * ______________________________________
 * Example of RDS/RBDS recevied blocks and their meaning:
 * BlockB BlockC BlockD type Comment/traduction:
 * 0x0268 0xE0CD 0x5269 (0A) Basic tuning and switching information only PS = u'Ri' @ index:0 PS = u'River '
 * 0x0269 0xE0CD 0x7665 (0A) Basic tuning and switching information only PS = u've' @ index:2 PS = u'Rive '
 * 0x026A 0xE0CD 0x7220 (0A) Basic tuning and switching information only PS = u'r ' @ index:4 PS = u'River '
 * 
 * 0x2262 0x4372 0x6F77 (2A) RadioText only RT1 = u'Crow' @ index:8  RT = u' Crow 
 * 0x2263 0x6E73 0x202D (2A) RadioText only RT1 = u'ns -' @ index:12 RT = u' Crowns -
 * 0x2264 0x204C 0x6966 (2A) RadioText only RT1 = u' Lif' @ index:16 RT = u' Crowns - Lif 
 * 0x2265 0x6573 0x6F6E (2A) RadioText only RT1 = u'eson' @ index:20 RT = u' Crowns - Lifeson
 */

/**
 * Types of group that are interessant for us:
 * Type:   A3 A2 A1 A0 B0 (Bits)
 * 0A      0  0  0  0  0  Basic Tuning and Switching Information only
 * 0B      0  0  0  0  1  Basic Tuning and Switching Information only
 * 2A      0  0  1  0  0  Radio Text only
 * 2B      0  0  1  0  1  Radio Text only
 * 
 */
typedef enum {
   GROUP_TYPE_0A = 0x00, /* PS (program Name) */
   GROUP_TYPE_0B = 0x01, /* PS (program Name) */
   GROUP_TYPE_2A = 0x04, /* Radio Text only */
   GROUP_TYPE_2B = 0x05  /* Radio Text only */
} GROUP_TYPE;

#define RT_GROUP_NBR_CHARS 4U  /* 4 characters are sent in one RT group */
#define RT_GROUP_MAX_INDEX 16U /* 4 characters * 16 = 64 characters. Maximum qty of characters allowed in RT free msg */
#define RT_GROUP_MAX_CHARS 64U /* 64 characters. Maximum qty of characters allowed in RT free msg */
#define PS_GROUP_MAX_CHARS 8U
#define PS_GROUP_NBR_CHARS 2U  /* 2 characters are sent in one RT group */

#define PTY_NUMBER         0x1F

/* Block B masks - RDS specific masking */
#define BLOCKB_GROUP_TYPE_MASK      0xF000
#define BLOCKB_GROUP_TYPE_POS       12U
#define BLOCKB_VERSION_MASK         0x0800
#define BLOCKB_VERSION_POS          11U
#define BLOCKB_TRAFFIC_PROG_MASK    0x0400
#define BLOCKB_TRAFFIC_PROG_POS     10U
#define BLOCKB_PROGRAM_TYPE_MASK    0x03E0
#define BLOCKB_PROGRAM_TYPE_POS     6U
#define BLOCKB_UNASSIGNED_BITS_MASK 0x001F   /* Are the index of text in case of group_type = RT */
#define BLOCKB_UNASSIGNED_BITS_POS  0U

typedef enum {
   RT_STATE_QUALITY_NONE = 0x00,
   RT_STATE_QUALITY_BAD,
   RT_STATE_QUALITY_MIDDLE,
   RT_STATE_QUALITY_GOOD
} RT_QUALITY_STATE;

/* For each group of RT received (group types 2A or 2B), 
 * describe state of those 4 characters. Allow us to say if we confirmed the 
 * redundancy of 4 characters received (same index, same value) and if we have a
 * full message (index cycle detected)
 */
typedef struct {
   RT_QUALITY_STATE _rt_group_state;
   uint8_t  _rt_group_chars[RT_GROUP_NBR_CHARS];   /* 4 characters per RT group */
} rds_rtGroup;

typedef struct {
   uint8_t _ptyID;
   char _ptyName[20];   /* @WARNING, change/remove magic number*/
} rds_ptyConverter;

typedef struct {
   /* All RT group received, maximum quantity is 16, otherwise, we received too many characters */
   rds_rtGroup       _rt_groups_rcvd[RT_GROUP_MAX_INDEX];/* RT Radio Text */
   uint8_t           _rt_index_cycle;                    /* Seems that 5th bit of unassigned bits in block B shows if we got a full RT cycle */
   uint8_t           _psName[PS_GROUP_MAX_CHARS];        /* PS name, eight characters max */
   uint16_t          _programIdentifer;                  /* PI  - identifies the station */
   uint8_t           _programType;                       /* PTY - identifies the station */
   RT_QUALITY_STATE  _qualityLevel;                      /* Desired quantity level of RT */
   bool              _radioText_valid;                   /* If user switched to another channel */
   bool              _verboseDecode;                     /* If we are in verbose mode */
} rds_decoder;

/*  */
typedef struct {
   uint16_t block_A;
   uint16_t block_B;
   uint16_t block_C;
   uint16_t block_D;
} rds_groupBlocks;

///////////////////// PUBLIC FUNCTIONS DEFINITIONS /////////////////////////////

/* Init decoder */

/**
 * @brief Initiate RDS decoder. I needs a wished quality state, which will fix
 * how many interation of same character set per block is needed, until considering
 * character set good enough. 
 * 
 * @param wishedRTState RT_QUALITY_STATE the wished minimal quality level
 * @param isVerbose     bool if we want to have something that prints RT decoded
 *                      even if the quality isn't good enough or not yet complete.
 */
void rdsDecoder_init(RT_QUALITY_STATE wishedRTState, bool isVerbose);

/**
 * @brief reset RDS RT message buffer and their quality levels
 * 
 */
void rdsDecoder_reset(void);

/**
 * @brief This function will sort the group, discard unnecessary data and sort 
 * important one. If full RT message is complete, it will return its size 
 * @TODO change API to tell if we finished PS message or RT message.
 * 
 * @param pointerToMsg  [out] pointer to message where to put RT message if finished
 * @param p_group       [in]  group of blocks freshly received from Si4703 to decode and sort
 * @return uint8_t      superior of 0 if message is finished, 0 if still on going
 */
uint8_t rdsDecoder_processNewGroup(uint8_t* pointerToMsg, rds_groupBlocks *p_group);

/**
 * @brief Retrieve RT as it is. A pointer to a full RT message size is needed
 * for it as we may still not know which size has this RT message. It will only
 * return all characters that matches required quality level or higher. Otherwise,
 * it will be filled with "SPACE" ASCII character.
 * 
 * @param pointerToMsg  [in] uint8_t* pointer to an array big enough to accept mgSize chars 
 * @param wishedRTState [in] RT_QUALITY_STATE quality level to reach
 * @param msgSize       [in] uint8_t size of message to be retreived
 */
void rdsDecoder_getRTMessage(uint8_t* pointerToMsg, RT_QUALITY_STATE wishedRTState, uint8_t msgSize);

/**
 * @brief Get Program Station name, will return only the pointer 
 *        to rdsDecoder internal memory PS name
 * 
 * @param buffPS  uint8_t* pointer to PS name
 * @return true 
 * @return false 
 */
bool rdsDecoder_getPS(uint8_t* buffPS);

#ifdef __cplusplus
}
#endif

#endif // _RDS_PARSER_H_
