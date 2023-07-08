/**
 * @file si470x_comm.c
 * @author Kureciplatek (galleej@gmail.com)
 * @brief   Gathers all hardware interface stuff of Si470x module. Basically a 
 *          HAL. Change this file to configure your desired pin layout and how
 *          to use I2C interface.
 * @version 0.1
 * @date 2023-02-14
 * 
 * @copyright Copyright (c) 2023 GPL license
 * 
 */

#include <stdio.h>
#include "hardware/i2c.h"
#include "si470x_comm.h"
#include "si470x_regs.h"

/* handle to i2c instance - use default instance of Raspberry Pico */
//static i2c_inst_t myInst = i2c_default;

/* -------------------------------------------------------------------------- */
/* ------------------------ READ/WRITE TO Si470x ---------------------------- */

void fm_si470x_initCommHW(void)
{
   printf("[FM][HW]  INIT comm HW\n");
   /* Si470X supports up to 400kHz SCLK frequency */
   i2c_init(i2c_default, SI470X_COMM_BAUDRATE);
   
   /* Output GPIOs */
   gpio_init(SI470X_COMM_PIN_SEN);
   gpio_set_dir(SI470X_COMM_PIN_SEN,   GPIO_OUT);

   gpio_init(SI470X_COMM_PIN_RESET);
   gpio_set_dir(SI470X_COMM_PIN_RESET, GPIO_OUT);

   gpio_init(SI470X_COMM_PIN_GPIO1);
   gpio_set_dir(SI470X_COMM_PIN_GPIO1, GPIO_OUT);

   /* Input GPIOs, for STC/RDS complete interrput */
//   gpio_init(SI470X_COMM_PIN_GPIO2);
//   gpio_set_dir(SI470X_COMM_PIN_GPIO2, GPIO_IN);

   /* I2C interface */
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c/bus_scan example requires a board with I2C pins
   puts("Default I2C pins were not defined");
#else
   gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
   gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
   gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
   gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
#endif
}

bool fm_si470x_readRegisters(uint16_t *regData, uint8_t regNbr) 
{
//   printf("Readregs called\n");
   bool retVal = false;

   /* Check number of register asked */
   if(SI470x_REG_MAX >= regNbr)
   {
      uint8_t bufData[SI4703_BYTES_FOR_STD_READ];
      uint8_t bufSize = 0x00;
      uint8_t bufSizeRegs = 0x00;
      
      /* Compute number of registers to read */
      if(regNbr < SI470x_REG_STATUSRSSI)
      {
         /* Add offset of registers 0xA to 0xF (0x06) to desired register
          because we need to wrap around */
         bufSizeRegs = regNbr + 0x07; /* @TODO modify magic number: RDS register size */
      }
      else
      {
         bufSizeRegs = regNbr - (SI470x_REG_STATUSRSSI - 0x01);
      }

      /* Convert number of registers to read to number of bytes */
      bufSize = bufSizeRegs * sizeof(uint16_t);

      /* I2C call */
      int result = i2c_read_blocking(i2c_default, SI4703_ADDR, bufData, bufSize, false);

      /* Transform back data into registers */
      if (result == (int)bufSize) 
      {
         uint16_t tempReg;
         uint8_t indexReg = 0x00; /* For all registers (16 bits words) */
         uint8_t indexBuf = 0x00; /* For all bytes of I2C, two bytes per reg */
         uint16_t wrongOrderRegisters[bufSizeRegs];

         /* Convert result to uint16_t register array */
         for (indexReg = 0; indexReg < bufSizeRegs ;indexReg++) 
         {
            /* At reading, Si470x sends back upper register byte first, then lower */
            tempReg  = bufData[indexBuf] << 8; // high register byte
            indexBuf++;
            tempReg |= bufData[indexBuf];      // low registe byte
            indexBuf++;
            wrongOrderRegisters[indexReg] = tempReg;
         }

         /* Restructure retrieved register array if we had to wrap around */
         if(regNbr < SI470x_REG_STATUSRSSI)
         {
//            printf("___ reorder with < RSSI ___\n");
            for(indexReg = 0; indexReg < regNbr; indexReg++)
            {
               /* Shift of 6 case because 0x0A to 0x0F are first in wrongOrderRegisters*/
               regData[indexReg] = wrongOrderRegisters[indexReg + 0x06];
//               printf("readed: %04x - %04x\n", regData[indexReg], wrongOrderRegisters[indexReg+ 0x06]);
            }
         }
         else
         {
//            printf("___ reorder with > RSSI ___\n");
            /* Then, add registers 0x0A to max wanted. Fill lower ones with 0x00 */
            for(indexReg = 0x00; indexReg < SI470x_REG_STATUSRSSI; indexReg++)
            {
               regData[indexReg] = 0x00;
            }
            for(indexReg = SI470x_REG_STATUSRSSI; indexReg < regNbr; indexReg++)
            {
               regData[indexReg] = wrongOrderRegisters[indexReg - SI470x_REG_STATUSRSSI];
            }
         }
         retVal = true;
      }
      else
      {
         printf("Error while reading regs on I2C\n");
      }
   }
   return retVal;
}

bool fm_si470x_writeRegisters(uint16_t *regData, uint8_t upperReg)
{
   bool retVal = false;
//   printf("Write regs called for reg: %d\n", upperReg);
   
   /* @WARNING checkt that I2C of Raspberry Pico RP2040 do send 8 bits of empty data before starting */

   /* Are register address write delimitations (see AN230) */
   if((upperReg >= SI470x_REG_POWERCFG) || (upperReg < SI470x_REG_MAX))
   {
      uint8_t buf[SI4703_BYTES_FOR_STD_WRITE];
      uint8_t index;
      uint8_t data_size       = (uint8_t)(upperReg * sizeof(uint16_t));
      uint16_t *pointerToData = regData + SI470x_REG_POWERCFG; /* Start at POWERCFG */
      int writeRes            = PICO_ERROR_GENERIC;

      /* Convert registers to bytes */
      for (index = 0; index < data_size;) 
      {
         buf[index++] = (uint8_t)(*pointerToData >> 8);     // high register
         buf[index++] = (uint8_t)(*pointerToData & 0x00FF); // low register
         pointerToData++;
      }
//      printf("Write to: %d\n", upperReg);
      
      /* Send to I2C - nonstop set to false, otherwise, error in Si470x internal register addr counter */
      writeRes = i2c_write_blocking(i2c_default, SI4703_ADDR, buf, data_size, false);
      if(writeRes == data_size)
      {
         retVal = true;
      }
      else
      {
         printf("[ERROR] I2C - Write regs\n");
      }
   }
   printf("Write done %d\n", upperReg);

   return retVal;
}



