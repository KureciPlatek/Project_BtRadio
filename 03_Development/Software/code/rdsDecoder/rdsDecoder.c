/**
 * @file rdsDecoder.c
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief   RDS decoder. Use it to decode the 4 RDS blocks reeived by Si470x module or
 *          any other FM receiver module (as RDS is standard). It is static and has an
 *          inernal buffer of memory to, step-by-step decode RDS data and place it in
 *          its rightful place.
 * @version 0.1
 * @date 2023-02-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "rdsDecoder.h"
#include <string.h>
#include <stdio.h>

/////////////////////////// GLOBAL VARIABLES ///////////////////////////////////
static rds_decoder _RDS_decoder;
//static rds_ptyConverter _RDS_all_pty[PTY_NUMBER];   /* Not yet supported */

/* Static functions to process data from blocks */
static void    rdsDecoder_processPS(uint16_t blockC, uint16_t blockD, uint8_t unassignedBits);
static uint8_t rdsDecoder_processRT(uint16_t blockC, uint16_t blockD, uint8_t unassignedBits);
//static bool rdsDecoder_processPTY(uint8_t programmeType);
static void    rdsDecoder_resetRT(void);
/* @brief Reset PS name from shadow registers */
static void    rdsDecoder_resetPS(void);


/* _________________ RDS INIT FUNCTIONS _________________ */

/* Init decoder */
void rdsDecoder_init(RT_QUALITY_STATE wishedRTState, bool isVerbose)
{
   printf("[FM][RDS] INIT RDS module\n");
   /* Cleanup memory */
   rdsDecoder_reset();
   rdsDecoder_resetPS();
   rdsDecoder_resetRT();   /* Pretty useless if I do a memset my dear */
   
   _RDS_decoder._radioText_valid = false;
   _RDS_decoder._verboseDecode   = isVerbose;
   _RDS_decoder._qualityLevel    = wishedRTState;
}

/* Reset decoder */
void rdsDecoder_reset(void)
{
   /* Everything set to 0 */
   memset(&_RDS_decoder, 0, sizeof(rds_decoder));
}

/* _________________ RDS DECODING FUNCTIONS _________________ */
uint8_t rdsDecoder_processNewGroup(uint8_t* pointerToMsg, rds_groupBlocks *p_group)
{
   uint8_t retVal          = 0x00;
   uint8_t rtFinished      = 0x00;
   uint8_t unassignedBits  = 0x00;
//   uint8_t programmeType   = 0x00;   /* Not implemented */
   uint8_t groupType       = 0x00;
   uint8_t trafficProgram  = 0x00;
   
   /* ____________________________ process blocks ___________________________ */
   /* ____ Block A: Program Identifier ____ */
   _RDS_decoder._programIdentifer = p_group->block_A;
   
   /* ____ Block B:group type: ____ */
   /* Get different info from block B: */
   unassignedBits = (p_group->block_B & BLOCKB_UNASSIGNED_BITS_MASK) >> BLOCKB_UNASSIGNED_BITS_POS;
   groupType      = (p_group->block_B & BLOCKB_GROUP_TYPE_MASK)      >> BLOCKB_GROUP_TYPE_POS;
//   programmeType  = (p_group->block_B & BLOCKB_PROGRAM_TYPE_MASK)    >> BLOCKB_PROGRAM_TYPE_POS;
   /* Process of version discarded, is not of use. Maybe in future */
   trafficProgram = (p_group->block_B & BLOCKB_TRAFFIC_PROG_MASK)    >> BLOCKB_TRAFFIC_PROG_POS;

   /* Process of RT and PS only if not Traffic Program */
   if(0x00 == trafficProgram)
   {
      /* Process PS (program Service Name) */
      if((GROUP_TYPE_0A == groupType) || (GROUP_TYPE_0B == groupType))
      {
         rdsDecoder_processPS(p_group->block_C, p_group->block_D, unassignedBits);
      }
      /* Process RT (Radio Text) */
      else if((GROUP_TYPE_2A == groupType) || (GROUP_TYPE_2B == groupType))
      {
         rtFinished = rdsDecoder_processRT(p_group->block_C, p_group->block_D, unassignedBits);

         /* If rtFinished > 0, RT decode finished, save it into pointerToMsg */
         if(0 < rtFinished)
         {
            rdsDecoder_getRTMessage(pointerToMsg, _RDS_decoder._qualityLevel, rtFinished);
            retVal = rtFinished;
         }
      }
      /* Here may be processed other kind of group type. Will be done in future */
   }
   return retVal;
}

static void rdsDecoder_processPS(uint16_t blockC, uint16_t blockD, uint8_t unassignedBits)
{
   /* @TODO add mechanism to check PS name integrity. Right now, do it funny and print what we have on screen */
   
   /* 2 characters received per group. Multiply index per 2 */
   uint8_t index = unassignedBits * PS_GROUP_NBR_CHARS;
   /* Seems that in block C, data is only 0xE0CD - @TODO find info about what is in Block C */
   _RDS_decoder._psName[index]         = (uint8_t)(blockD & 0xFF00) >> 8;
   _RDS_decoder._psName[index + 0x01]  = (uint8_t)(blockD & 0x00FF);

   if(0xE0CD != blockC)
   {
      printf("PS block C value: %02X\n", blockC);
   }
}

static uint8_t rdsDecoder_processRT(uint16_t blockC, uint16_t blockD, uint8_t unassignedBits)
{
   uint8_t sizeOfRTmessage = 0x00;

   /* 4 characters received per group. But we keep first 4 chars per 4. Assmbly will be at the end when message finished */
   /* It is the index of RT message group. We receive RT group per RT group a 
    * bunch of 4 chars, and an index. This index is only on four first bits. 
    * 5th is for index cycle (if bit changed, one RT cycle is done) */
   uint8_t rt_group_index        = (unassignedBits & 0x0F);
   uint8_t rt_group_index_cycle  = (unassignedBits & 0x10) >> 4;
   
   uint8_t character_CA = (uint8_t)(blockC & 0xFF00) >> 8; /* 1st character */
   uint8_t character_CB = (uint8_t)(blockC & 0x00FF);      /* 2nd character */
   uint8_t character_DA = (uint8_t)(blockD & 0xFF00) >> 8; /* 3rd character */ 
   uint8_t character_DB = (uint8_t)(blockD & 0x00FF);      /* 4th character */
   
   /* Prepare pointers to place where decoded ASCII characters must go */
   uint8_t* pCharacter0 = &_RDS_decoder._rt_groups_rcvd[rt_group_index]._rt_group_chars[0];
   uint8_t* pCharacter1 = &_RDS_decoder._rt_groups_rcvd[rt_group_index]._rt_group_chars[1];
   uint8_t* pCharacter2 = &_RDS_decoder._rt_groups_rcvd[rt_group_index]._rt_group_chars[2];
   uint8_t* pCharacter3 = &_RDS_decoder._rt_groups_rcvd[rt_group_index]._rt_group_chars[3];

   RT_QUALITY_STATE actualLineState = _RDS_decoder._rt_groups_rcvd[rt_group_index]._rt_group_state;
   
   /* No characters received at this line yet. Fill it */
   if(RT_STATE_QUALITY_NONE == actualLineState)
   {
      *pCharacter0 = character_CA;
      *pCharacter1 = character_CB;
      *pCharacter2 = character_DA;
      *pCharacter3 = character_DB;
      
      _RDS_decoder._rt_groups_rcvd[rt_group_index]._rt_group_state++; /* Increment quality */
   }
   /* Already received characters at this line, check if they are the same */
   else if (actualLineState < _RDS_decoder._qualityLevel)
   {
      if( (*pCharacter0 == character_CA)
       && (*pCharacter1 == character_CB)
       && (*pCharacter2 == character_DA)
       && (*pCharacter3 == character_DB))
      {
         _RDS_decoder._rt_groups_rcvd[rt_group_index]._rt_group_state++; /* Increment quality */
      }
      else
      {
         /* @TODO it may be a new message. If we are in state 
         AUTOCORRECTING from Si470x, then reset message, otherwise, just discard this element */
         
         /* Save new RT data */
         *pCharacter0 = character_CA;
         *pCharacter1 = character_CB;
         *pCharacter2 = character_DA;
         *pCharacter3 = character_DB;
         
         _RDS_decoder._rt_groups_rcvd[rt_group_index]._rt_group_state++; /* Increment quality */
      }
   }
   /* Else, quality level already reached for this line (group) */

   /* RT complete only if 5th bit of blockB->unassigned_bits changes its state */
   if(rt_group_index_cycle != _RDS_decoder._rt_index_cycle)
   {
      /* Save actual index cycle state */
      _RDS_decoder._rt_index_cycle = rt_group_index_cycle;
      
      /* Check that all received RT group quality meet required quality level */
      uint8_t index = 0;
      while (_RDS_decoder._rt_groups_rcvd[index]._rt_group_state >= _RDS_decoder._qualityLevel) {
         index++;
      }
      
      if(index == RT_GROUP_MAX_INDEX)
      {
         /* Radio Text message complete, return size of RT message in multiple of 4 */
         /* When bit 5 changes, we reached maximum index of RT message */
         sizeOfRTmessage = rt_group_index;
      }
   }

   return sizeOfRTmessage;
}

void rdsDecoder_getRTMessage(uint8_t* pointerToMsg, RT_QUALITY_STATE wishedRTState, uint8_t msgSize)
{
   uint8_t indexLines = 0U;
   uint8_t charStringIndex = 0U;
   
   for (;indexLines < msgSize; indexLines++)
   {
      if(_RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_state >= wishedRTState)
      {
         /* 4 characters per line, move 4 per 4 */
         charStringIndex = indexLines * RT_GROUP_NBR_CHARS;
         *(pointerToMsg + (charStringIndex))     = _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_chars[0];
         *(pointerToMsg + (charStringIndex + 1)) = _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_chars[1];
         *(pointerToMsg + (charStringIndex + 2)) = _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_chars[2];
         *(pointerToMsg + (charStringIndex + 3)) = _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_chars[3];
      }
      else /* Fill with blank */
      {
         *(pointerToMsg + (charStringIndex))     = 0x20; /* Fill with SPACE ASCII character */
         *(pointerToMsg + (charStringIndex + 1)) = 0x20; /* Fill with SPACE ASCII character */
         *(pointerToMsg + (charStringIndex + 2)) = 0x20; /* Fill with SPACE ASCII character */
         *(pointerToMsg + (charStringIndex + 3)) = 0x20; /* Fill with SPACE ASCII character */
      }
   }
}

static void rdsDecoder_resetRT(void)
{
   uint8_t indexLines = 0U;
   /* Reset only RT messages */
   for (;indexLines < RT_GROUP_MAX_INDEX; indexLines++)
   {
      _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_chars[0] = 0x00;
      _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_chars[1] = 0x00;
      _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_chars[2] = 0x00;
      _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_chars[3] = 0x00;
      _RDS_decoder._rt_groups_rcvd[indexLines]._rt_group_state = RT_STATE_QUALITY_NONE;
   }
   _RDS_decoder._rt_index_cycle = 0x00;
   
   /* Do not use memset(), as it resets all bytes of memory with the size of argument
    * Siez of an enumerate is unknown
    */
}

static void rdsDecoder_resetPS(void)
{
   uint8_t index = 0;
   for(;index < PS_GROUP_MAX_CHARS; index++)
   {
      _RDS_decoder._psName[index] = 0x20; /* Fill with space character */
   }
}

bool rdsDecoder_getPS(uint8_t* buffPS)
{
   /* Send back pointer to frist character of string */
   buffPS = &_RDS_decoder._psName[0];

   printf("Got: %c", *buffPS);

   /* @TODO adapt if PS parsing not yet finished */
   return true;
}