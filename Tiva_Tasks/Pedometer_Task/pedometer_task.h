


#ifndef MAIN_H_
#define MAIN_H_

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
#include "driverlib/interrupt.h"

#include "utils/uartstdio.h"
#include "inc/tm4c1294ncpdt.h"

#include "FreeRTOS.h"

#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* Macros section */

#define SYSTEM_CLOCK                        120000000U /* System clock rate, 120 MHz */
#define TASK_BUFFER                         1024
#define SLAVE_ADDR                          0b1101010
#define SLAVE_ADDR_WRITE                    0b11010100
#define SLAVE_ADDR_READ                     0b11010101

#define FUNC_CFG_ACCESS_REG                 0X01
#define FUNC_CFG_ACCESS_REG_VALUE           0X01 /* To enable access to embedded function config reg. */
#define WHO_AM_I_REG                        0X0F
#define STEP_COUNT_L_REG                    0X4B
#define STEP_COUNT_H_REG                    0X4C

#define I2C_BASE                            I2C0_BASE
#define LSM6DS3_SENSOR_ADDR                 0x6B

/* Pedometer Configuration Registers */
#define PEDOMETER_SENSOR_CTRL9_XL_REG       0x18
#define PEDOMETER_SENSOR_LIN_ACCL_CFG_VAL   0x38

#define PEDOMETER_SENSOR_CTRL1_X1_REG       0x10    // CTRL1_X1 register
#define PEDOMETER_SENSOR_ACC_ODR_CFG_VAL    0x20

#define PEDOMETER_SENSOR_CTRL10_C_REG       0x19
#define PEDOMETER_SENSOR_FUNC_EN_CFG_VAL    0x3C
#define PEDOMETER_SENSOR_STEP_CNT_RST_CFG_VAL   0x3E

#define PEDOMETER_SENSOR_TAP_CFG_REG        0x58
#define PEDOMETER_SENSOR_PEDO_EN_CFG_VAL    0x40

#define PEDOMETER_SENSOR_INT_CTRL1_REG      0x0D
#define PEDOMETER_SENSOR_INT1_STEP_DETECT_CFG_VAL  0x80

#define PEDOMETER_SENSOR_STATUS_REG         0x1E

#define PEDOMETER_SENSOR_STEP_COUNTER_L_REG 0x4B
#define PEDOMETER_SENSOR_STEP_COUNTER_H_REG 0x4C

#define PEDOMETER_COUNT_PRINT_DELAY         300000UL

#define MAX_MSG_LEN
#define QUEUE_LENGTH                        10


/* Task Defines */
#define TASK_PEDOMETER                      0x1
#define TASK_HEART_RATE                     0x2

/* Log type defines */
#define LOG_TYPE_DATA                       0x1
#define LOG_TYPE_ERROR                      0x2
#define LOG_TYPE_REQUEST                    0x3
#define LOG_TYPE_RESPONSE                   0x4

/* Log level defines */
#define LOG_LEVEL_INFO                      0x1
#define LOG_LEVEL_STARTUP                   0x2
#define LOG_LEVEL_SHUTDOWN                  0x3
#define LOG_LEVEL_CRITICAL                  0x4

SemaphoreHandle_t xUARTSemaphore;
SemaphoreHandle_t xQueueMutex;
SemaphoreHandle_t xPedometerDataAvail;
QueueHandle_t xQueue;

/* System clock rate in Hz */
uint32_t g_ui32SysClock;

uint32_t gui32IsrCounter;

/* Socket message structure */
typedef struct
{
    uint32_t ui32LogLevel;
    uint32_t ui32LogType;
    uint32_t ui32SourceId ;
    uint32_t ui32StepCount;
} sock_msg;

/**
 *  @brief UART Init
 *
 *  This function will perform the system level initialization of UART
 *
 *  @param      void
 *
 *  @return     void
 */

void UART0_Init(void);

void I2C0_Init(void);

/**
 *  @brief Create primary tasks
 *
 *  This function will create the queue that will be used for communication
 *  between the third task handling LEDs and UART and the second task that
 *  notifies the third task of the LOG_STRING event.
 *
 *  @param      void
 *
 *  @return     pdPASS  : If task creation is successful
 *              pdFAIL  : If task creation fails
 */
BaseType_t CreateTasks(void);

/**
 *  @brief UART Send function
 *
 *  This function will send the data in @param pui8MsgStr to UART
 *
 *  @param      pui8MsgStr  : pointer to message
 *
 *  @return     void
 */
void UARTSend(uint8_t *pui8MsgStr);

void vPedometerTaskInit(void);

void vPedometerTask( void *pvParameters );

void vWritePedometerSensorRegister(uint32_t ui32RegToWrite, uint32_t ui32RegVal);

uint32_t vReadPedometerSensorregister(uint32_t ui32RegToRead);

void vUARTTask(void *pvParameters);

int QueueCreate(void);

void UART7Init(void);

//void UARTSendToBBG(char *pucBuffer);
void UARTSendToBBG(char *pucBuffer, uint32_t ui32BufLen);

void xPedometerStepCountIntHandler(void);

#endif /* MAIN_H_ */
