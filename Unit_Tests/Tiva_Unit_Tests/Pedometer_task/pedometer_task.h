/*
 * pedometer_task.h
 *
 */

#ifndef PEDOMETER_TASK_H_
#define PEDOMETER_TASK_H_

/* Headers Section */
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* System clock rate in Hz */
uint32_t g_ui32SysClock;

/* Macros section */

#define SYSTEM_CLOCK    120000000U /* System clock rate, 120 MHz */
#define TASK_BUFFER 1024
#define SLAVE_ADDR  0b1101010
#define SLAVE_ADDR_WRITE    0b11010100
#define SLAVE_ADDR_READ 0b11010101

#define FUNC_CFG_ACCESS_REG 0X01
#define FUNC_CFG_ACCESS_REG_VALUE 0X01 /* To enable access to embedded function config reg. */
#define WHO_AM_I_REG 0X0F
#define STEP_COUNT_L_REG 0X4B
#define STEP_COUNT_H_REG 0X4C


void InitI2C0(void);


#endif /* PEDOMETER_TASK_H_ */
