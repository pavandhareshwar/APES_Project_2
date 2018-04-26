/*
 * humidity_task.h
 *
 */

#ifndef HUMIDITY_TASK_H_
#define HUMIDITY_TASK_H_

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

#define HUMIDITY_SENSOR_ADDR    0x40
#define TRIGGER_HUMIDITY_MEASUREMENT    0xF5
#define WRITE_USER_REG  0xE6
#define READ_USER_REG  0xE7
#define DEFAULT_VALUE 0x01
#define I2C_BASE                            I2C0_BASE


void I2C0_Init(void);


#endif /* HUMIDITY_TASK_H_ */
