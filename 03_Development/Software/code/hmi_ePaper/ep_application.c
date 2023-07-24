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


bool ep_init(void)
{
   bool retVal = true;
   printf("Init epaper display\r\n");
   
   if(0 != DEV_Module_Init())
   {
      retVal = false;
   }
   if(false != retVal)
   {
      printf("e-Paper Init and Clear...\r\n");
      EPD_5in83_V2_Init();
      EPD_5in83_V2_Clear();
      /* Wait for screen to start up */
      DEV_Delay_ms(500);
      
      /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
      UWORD Imagesize = ((EPD_5in83_V2_WIDTH % 8 == 0)? (EPD_5in83_V2_WIDTH / 8 ): (EPD_5in83_V2_WIDTH / 8 + 1)) * EPD_5in83_V2_HEIGHT;
  
      if((ep_imageBUffer = (UBYTE *)malloc(Imagesize)) == NULL) 
      {
         printf("Failed to apply for black memory...\r\n");
         retVal = false;
      }
      else
      {
         printf("Paint_NewImage\r\n");
         Paint_NewImage(ep_imageBUffer, EPD_5in83_V2_WIDTH, EPD_5in83_V2_HEIGHT, ROTATE_270, WHITE); 
      }
   }
   return retVal;
}

bool ep_write(EPAPER_PLACE place, uint8_t line, char * ptrToString)
{
   Paint_SelectImage(ep_imageBUffer);
   Paint_Clear(WHITE);
   Paint_DrawString_EN(10, 10, ptrToString, &Font24, WHITE, BLACK);
   EPD_5in83_V2_Display(ep_imageBUffer);
   
   printf("Goto Sleep...\r\n");
   EPD_5in83_V2_Sleep();

   return true;
}

bool ep_deactivate(void)
{
   free(ep_imageBUffer);
   ep_imageBUffer = NULL;
   DEV_Delay_ms(2000);//important, at least 2s
   // close 5V
   printf("close 5V, Module enters 0 power consumption ...\r\n");
   DEV_Module_Exit();

   return true;
}
