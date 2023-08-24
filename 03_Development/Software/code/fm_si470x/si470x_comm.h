/**
 * @file    si470x_comm.h
 * @author  Kureciplatek (galleej@gmail.com)
 * @brief   Gathers all hardware interface stuff of Si470x module. Basically a 
 *          HAL. Change this file to configure your desired pin layout and how
 *          to use I2C interface.
 * @version 0.1
 * @date 2023-02-14
 * 
 * @copyright Copyright (c) 2023 GPL license
 * 
 */

#ifndef _SI470X_COMM_H_
#define _SI470X_COMM_H_

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define SI470X_COMM_PIN_RESET 2
#define SI470X_COMM_PIN_SEN   3
#define SI470X_COMM_PIN_SDIO  4  // Own project I2C_SDA_PIN
#define SI470X_COMM_PIN_SCLK  5  // Own project I2C_SCL_PIN
#define SI470X_COMM_PIN_GPIO1 7
// #define SI470X_COMM_PIN_GPIO2 6

#define SI470X_COMM_BAUDRATE  400000   /* In Hz */

typedef enum {
   PIN_LOW,
   PIN_HIGH
} PIN_STATE;

/**
 * @brief write registers on Si470x until a special register.
 * Si470x has specific way to write its registers. It starts with reg address SI470x_REG_POWERCFG (0x02)
 * and carries on until either we reached upperReg, or max register address (0x0F).
 * Si4703 always start its write with register POWERCFG (0x02).
 * 
 * @param regs      uint16_t*   pointer to data to write into registers
 * @param upperReg  uint8_t     maximum register address to write to 
 * @return true     i2c_write_blocking success
 * @return false    i2c_write_blocking failure
 */
bool si470x_comm_writeRegisters(uint16_t *regData, uint8_t upperReg);

/**
 * @brief Read registers of Si4703. Read is special as it start with address 0x0A
 * in internal memory of Si4703. Maximum reg address is 0x0F, so if we need to read
 * registers 0x00 to 0x09 too, we must carry on with reading, beyond 0x0F. But Si4703 will
 * automatically wrap around and read registers 0x00 to 0x09 too.
 * 
 * @warning If you execute a read "up to" womething between 0x0A and 0x0F, retrieved
 * data will only be about those values! Means registers 0x00 to 0x09 will contain 0x00
 * 
 *                                        |-> Starts here       -->
 * 0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7 0x8 0x9|0xA 0xB 0xC 0xD 0xE 0xF 
 * --> carries on here (if needed)        |
 * 
 * @param regData       uint16_t* pointer to buffer to save data retreived
 * @param regNbr        uint8_t number of byte to retrieve from Si4703
 * @return true         if read successed (from i2c_read_blocking() call)
 * @return false        if read failed (from i2c_read_blocking() call)
 */
bool si470x_comm_readRegisters(uint16_t *regData, uint8_t regNbr);

/**
 * @brief Init I2C communication for Si470x module with parameters defined in
 * preprocessor. Change preprocessor to adapt it to wished I2C communication. 
 * 
 */
void si470x_comm_initHW(void);


////////////////////////// GPIOs MANUAL CONTROL //////////////////////////////

/**
 * @brief Set SEN of Si470x interface to a wished state
 * 
 * @param state  PIN_STATE on or off?
 */
static inline void fm_si470x_setSEN(PIN_STATE state)
{
   if(PIN_LOW == state) {
      gpio_put(SI470X_COMM_PIN_SEN, 0);
   }
   else {
      gpio_put(SI470X_COMM_PIN_SEN, 1);
   }
}

/**
 * @brief Set GPIO1 of Si470x interface to a wished state
 * 
 * @param state  PIN_STATE on or off?
 */
static inline void fm_si470x_setGPIO1(PIN_STATE state)
{
   if(PIN_LOW == state) {
      gpio_put(SI470X_COMM_PIN_GPIO1, 0);
   }
   else {
      gpio_put(SI470X_COMM_PIN_GPIO1, 1);
   }
}

/**
 * @brief Set RST of Si470x interface to a wished state
 * 
 * @param state  PIN_STATE on or off?
 */
static inline void fm_si470x_setRST(PIN_STATE state)
{
   if(PIN_LOW == state) {
      gpio_put(SI470X_COMM_PIN_RESET, 0);
   }
   else {
      gpio_put(SI470X_COMM_PIN_RESET, 1);
   }
}


#endif // _SI470X_COMM_H_
