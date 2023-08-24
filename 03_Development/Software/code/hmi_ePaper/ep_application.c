/**
 * @file    ep_application.c
 * @author  Jeremie Gallee (galleej@gmail.com)
 * @brief 
 * @version 0.1
 * @date    2023-07-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ep_application.h"
#include "EPD_5in83_V2.h"

/* Screen buffer image */
static UBYTE *ep_imageBUffer;

/* uint8_t coordinates_X uint8_t coordinates_Y sFONT desiredFont */   
static epaperConfig _radioScreenConfig[EPAPER_PLACE_MAX] = {
   {10, 40, &Font24},   /* EPAPER_PLACE_ACTIVEMODE   */
   {10, 100, &Font24},  /* EPAPER_PLACE_BT_STATUS    */
   {10, 220, &Font24},  /* EPAPER_PLACE_BT_TRACK     */
   {10, 400, &Font24}   /* EPAPER_PLACE_FM_FAVORITE  */
};

bool ep_init(void)
{
   bool retVal = true;
   printf("[EP][API] Start Iinit epaper display\r\n");
   
   if(0 != DEV_Module_Init())
   {
      retVal = false;
   }
   if(false != retVal)
   {
      printf("[EP][API] ePaper Init and Clear\r\n");
      EPD_5in83_V2_Init();
      EPD_5in83_V2_Clear();
      /* Wait for screen to start up */
      DEV_Delay_ms(500);
      
      /* @todo you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
      UWORD Imagesize = ((EPD_5in83_V2_WIDTH % 8 == 0)? (EPD_5in83_V2_WIDTH / 8 ): (EPD_5in83_V2_WIDTH / 8 + 1)) * EPD_5in83_V2_HEIGHT;
  
      if((ep_imageBUffer = (UBYTE *)malloc(Imagesize)) == NULL) 
      {
         printf("[EP][API] Failed to save enough memory for e-paper buffer image\n");
         retVal = false;
      }
      else
      {
         /* Clean memory at this place */
         ep_cleanImageBUffer();
         printf("[EP][API] Paint_NewImage\r\n");
         Paint_NewImage(ep_imageBUffer, EPD_5in83_V2_WIDTH, EPD_5in83_V2_HEIGHT, ROTATE_270, WHITE); 
      }
   }
   return retVal;
}

bool ep_write(EPAPER_PLACE place, uint8_t line, char * ptrToString, bool flush)
{
#if 0
   printf("[EP][API] ep_write called\n");
   Paint_SelectImage(ep_imageBUffer);

   UWORD Y_startClean = _radioScreenConfig[place].coordinates_Y + (line * EPAPER_CHARS_PER_LINE);
   UWORD Y_start = Y_startClean + 3;
   UWORD Y_end = Y_startClean + EPAPER_CHARS_PER_LINE;

   printf("[EP][API] Clean Pixels %d to %d + write at: %d\n", Y_startClean, Y_end, Y_start);
   //void Paint_ClearWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color)
   Paint_ClearWindows(0, Y_startClean, 480, Y_end, WHITE);

   /* void Paint_DrawString_EN(UWORD Xstart, UWORD Ystart, const char * pString,            */
   /*                          sFONT* Font, UWORD Color_Foreground, UWORD Color_Background) */
   Paint_DrawString_EN(_radioScreenConfig[place].coordinates_X, 
                       Y_start,
                       ptrToString,
                       _radioScreenConfig[place].desiredFont,
                       WHITE, BLACK);
   
   if(true == flush)
   {
      ep_flush();
   }
   
   /* Deep sleep which requires hard ward reset assertion to be functional again. Deactivate */
//   EPD_5in83_V2_Sleep();
#endif
   return true;
}

void ep_flush(void)
{
#if 0
   EPD_5in83_V2_Display(ep_imageBUffer);
#endif
}

bool ep_deactivate(void)
{
   EPD_5in83_V2_Clear();

   free(ep_imageBUffer);
   ep_imageBUffer = NULL;

   /* 2 seconds timout at least are required before exit module (required from manufacturer) */
   DEV_Delay_ms(2000);
   /* Deactivate power supply */
   printf("[EP][API] ePaper dactivated\n");
   DEV_Module_Exit();

   return true;
}

void ep_cleanImageBUffer(void)
{
   uint32_t indexToClean = 0x00; 
   // 648*480 = 311040, need an uint32_t
   uint32_t sizeOfStuff = ((EPD_5in83_V2_WIDTH % 8 == 0)? (EPD_5in83_V2_WIDTH / 8 ): (EPD_5in83_V2_WIDTH / 8 + 1)) * EPD_5in83_V2_HEIGHT;
   for(indexToClean = 0x00; indexToClean < sizeOfStuff; indexToClean++)
   {
      *(ep_imageBUffer + indexToClean) = 0xFF;
   }
}
