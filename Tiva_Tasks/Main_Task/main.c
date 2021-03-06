/*******************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/22/2018
* File:         main.c
* Description:  Source file containing the functionality and implementation
*               of the freertos functionality for pedometer sensor and
*               humidity sensor along with LWIP/UART as communication interfaces
********************************************************************************/

/* Headers Section */
#include "main.h"

/* System clock rate in Hz */
uint32_t g_ui32SysClock;

int main(void)
{
    int32_t  i32RetVal = -1;

    /* ---------------------------------------------------------------------------------------------------------- */
    /* Configure system clock to be at the max (120 MHz) */
    g_ui32SysClock = MAP_SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ  /* Frequency of external crystal */
                                                  | SYSCTL_OSC_MAIN  /* Use an external crystal or oscillator */
                                                  | SYSCTL_USE_PLL   /* select the PLL output as the system clock */
                                                  | SYSCTL_CFG_VCO_480, /* set the PLL VCO output to 480-MHz */
                                                  SYSTEM_CLOCK /* requested processor frequency */
    );

    /* ---------------------------------------------------------------------------------------------------------- */

    /* Initializing the UART for logging */
    UART0_Init();

#ifdef UART_ENABLE
    /* Communication peripheral between BBG and TIVA */
    UART7Init();
#else

    PinoutSet(true,false);

    /* Enabling the systick handler for LWIP */
    MAP_SysTickPeriodSet(g_ui32SysClock/ SYSTICKHZ);
    MAP_SysTickEnable();
    MAP_SysTickIntEnable();

    /* Delay for Systick handler to initialize */
    SysCtlDelay(1000);

    /* LWIP socket initialize */
    lwip_socket_init();

    while(!gbReadyToSend);

#endif

    /* Initializing LED's */
    LED_Init();

    /* Initilaizing I2C */
    I2C0_Init();

    /* Creating queue for logging */
    QueueCreate();

    gui32CheckHbPedTask = 0;

    /* Check for startup test of pedometer and humidity sensor */
    int startupTestRetVal = PerformSysStartUpTest();
    if (startupTestRetVal == -1)
    {
        UARTSend("Startup Test Failed. Exiting system\r\n");
        /* Sending failure case to BBG */
        vLogData(LOG_LEVEL_STARTUP, LOG_TYPE_ERROR, TASK_MAIN, FAILURE );
        GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_0,1);
        exit(1);
    }
    else
    {
        /* Sending success case to BBG */
        vLogData(LOG_LEVEL_STARTUP, LOG_TYPE_DATA, TASK_MAIN, SUCCESS );
    }

    if (CreateTasks() == pdPASS)
    {
        /* Start the scheduler */
        vTaskStartScheduler();

        for(;;);
    }
    else
    {
        /* Sending success case to BBG */
        vLogData(LOG_LEVEL_CRITICAL, LOG_TYPE_ERROR, TASK_MAIN, FAILURE );

        UARTSend("Task creation failed\r\n");
        i32RetVal = -1;
    }

	return i32RetVal;
}

#ifndef UART_ENABLE

void vLWIPTask( void *pvParameters )
{
    sock_msg xSockMsg;

    for(;;)
    {
        memset((void*)&xSockMsg,0,sizeof(xSockMsg));

        while(uxQueueSpacesAvailable(xQueue) != QUEUE_LENGTH)
        {
            if(xSemaphoreTake(xQueueMutex,portMAX_DELAY)){
                if( xQueueReceive( xQueue, &xSockMsg, portMAX_DELAY ) != pdPASS ){
                    UARTSend("\ERROR:Queue Receive\r\n");
                }
                else{
                    LWIPSendToBBG((uint8_t *)&xSockMsg, sizeof(xSockMsg));
                }
                xSemaphoreGive(xQueueMutex);
            }
        }
    }
}

void LWIPSendToBBG(uint8_t *pui8Buffer, uint32_t ui32Len)
{
    tcp_write(pcb, pui8Buffer, ui32Len, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
}

#endif

void LED_Init()
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    /* PN0 and PN1 are used for USER LEDs. */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    MAP_GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1,
                             GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

    /* Default the LEDs to OFF. */
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);
}

void I2C0_Init(void)
{
    /* Enable the GPIO Peripheral used by the I2C0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    /* Configure GPIO pins for I2C0 */
    MAP_GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    MAP_GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    /* Configure GPIO pin PB2 as SDA and PB3 as SCL */
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    MAP_GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    /* Disable-reset-enable the I2C0 peripheral */
    MAP_SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C0);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    /* Configure master module */
    MAP_I2CMasterInitExpClk(I2C_BASE, SYSTEM_CLOCK, true);
}

int PerformSysStartUpTest(void)
{
    /* Check for pedometer sensor */
    int pedometerSensorTestVal = vTestPedometerSensor();
    if (pedometerSensorTestVal == -1)
    {
        UARTSend("Pedometer Startup test failed\r\n");
        return -1;
    }

    /* Check for humidity sensor */
    int humiditySensorTestVal = vTestHumiditySensor();
    if (humiditySensorTestVal == -1)
    {
        UARTSend("Humidity Startup test failed\r\n");
        return -1;
    }

    return 0;
}

int QueueCreate(void)
{
    /* Creating queue for logging */
    xQueue = xQueueCreate(QUEUE_LENGTH, sizeof(sock_msg));
    if (xQueue != NULL)
    {
        UARTSend("Queue created successfully\r\n");

        xQueueMutex = xSemaphoreCreateMutex();
    }
    else
    {
        UARTSend("Queue creation failed\r\n");
        return -1;
    }

    return 0;
}

BaseType_t CreateTasks(void)
{
    BaseType_t xRetVal = pdTRUE;

#ifdef UART_ENABLE

    if(xTaskCreate(vPedometerTask, "Pedometer_Task", TASK_BUFFER_SIZE, NULL, 1, &hdlPedTask) == pdFALSE)
    {
        UARTSend("Pedometer Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }

    if(xTaskCreate(vHumidityTask, "Humidity_Task", TASK_BUFFER_SIZE, NULL, 1, &hdlHumTask) == pdFALSE)
    {
        UARTSend("Humidity Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }

    if(xTaskCreate(vMainTask, "MainTask", TASK_BUFFER_SIZE, NULL, 1, &hdlMainTask) == pdFALSE)
    {
        UARTSend("Main Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }

    if(xTaskCreate(vUARTWriterTask, "UARTWriterTask", TASK_BUFFER_SIZE, NULL, 1, &hdlUARTWriterTask) == pdFALSE)
    {
        UARTSend("UART Writer Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }

    if(xTaskCreate(vUARTReaderTask, "UARTReaderTask", TASK_BUFFER_SIZE, NULL, 1, &hdlUARTReaderTask) == pdFALSE)
    {
        UARTSend("UART Writer Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }

#else

    if(xTaskCreate(vPedometerTask, "Pedometer_Task", TASK_BUFFER_SIZE, NULL, 2, &hdlPedTask) == pdFALSE)
    {
        UARTSend("Pedometer Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }

    if(xTaskCreate(vLWIPTask, "LWIP_Task", TASK_BUFFER_SIZE, NULL, 1, NULL) == pdFALSE)
    {
        UARTSend("LWIP Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }
#endif

    return xRetVal;
}

void vMainTask( void *pvParameters )
{
    static heartbeat_count = 0;

    xHbPedTaskCheckSem = xSemaphoreCreateMutex();
    xHbPedTaskValSem = xSemaphoreCreateMutex();

    xHbHumTaskCheckSem = xSemaphoreCreateMutex();
    xHbHumTaskValSem = xSemaphoreCreateMutex();

    xHbUARTWrTaskCheckSem = xSemaphoreCreateMutex();
    xHbUARTWrTaskValSem = xSemaphoreCreateMutex();

    xHbUARTRdTaskCheckSem = xSemaphoreCreateMutex();
    xHbUARTRdTaskValSem = xSemaphoreCreateMutex();

    while (1)
    {
        /* Send heartbeat request to each sub-tasks and get a response */

        /* Check heartbeat from pedometer task */
        if (xSemaphoreTake(xHbPedTaskCheckSem, portMAX_DELAY) == pdTRUE) {
            gui32CheckHbPedTask = 1;
            xSemaphoreGive(xHbPedTaskCheckSem);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        if (xSemaphoreTake(xHbPedTaskValSem, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (gui32PedTaskHb == 1)
            {
                heartbeat_count++;
                UARTSend("Pedometer task alive\r\n");
            }
            else
            {
                gui32PedTaskUnAliveCount++;
                if (gui32PedTaskUnAliveCount > TASK_UNALIVE_CNT_UPPER_LIMIT){
                    /* Sending heart beat failure case to BBG */
                    vLogData(LOG_LEVEL_CRITICAL, LOG_TYPE_HEARTBEAT, TASK_MAIN, PED_HB_FAILURE );
                    UARTSend("Report pedometer task's unalive state to BBG\r\n");
                }
            }
            xSemaphoreGive(xHbPedTaskValSem);
        }

        /* Check heartbeat from humidity task */
        if (xSemaphoreTake(xHbHumTaskCheckSem, portMAX_DELAY) == pdTRUE) {
            gui32CheckHbHumTask = 1;
            xSemaphoreGive(xHbHumTaskCheckSem);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        if (xSemaphoreTake(xHbHumTaskValSem, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (gui32HumTaskHb == 1)
            {
                heartbeat_count++;
                UARTSend("Humidity task alive\r\n");
            }
            else
            {
                gui32HumTaskUnAliveCount++;
                if (gui32HumTaskUnAliveCount > TASK_UNALIVE_CNT_UPPER_LIMIT){
                    /* Sending heart beat failure case to BBG */
                    vLogData(LOG_LEVEL_CRITICAL, LOG_TYPE_HEARTBEAT, TASK_MAIN, HUM_HB_FAILURE );
                    UARTSend("Report humidity task's unalive state to BBG\r\n");
                }
            }
            xSemaphoreGive(xHbHumTaskValSem);
        }

        /* Check heartbeat from UART writer task */
        if (xSemaphoreTake(xHbUARTWrTaskCheckSem, portMAX_DELAY) == pdTRUE) {
            gui32CheckHbUARTWrTask = 1;
            xSemaphoreGive(xHbUARTWrTaskCheckSem);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        if (xSemaphoreTake(xHbUARTWrTaskValSem, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (gui32UARTWrTaskHb == 1)
            {
                heartbeat_count++;
                UARTSend("UART writer task alive\r\n");
            }
            else
            {
                gui32UARTWrTaskUnAliveCount++;
                if (gui32UARTWrTaskUnAliveCount > TASK_UNALIVE_CNT_UPPER_LIMIT){
                    /* Sending heart beat failure case to BBG */
                    vLogData(LOG_LEVEL_CRITICAL, LOG_TYPE_HEARTBEAT, TASK_MAIN, UW_HB_FAILURE );
                    UARTSend("Report UART writer task's unalive state to BBG\r\n");
                }
            }
            xSemaphoreGive(xHbUARTWrTaskValSem);
        }

        /* Check heartbeat from UART reader task */
        if (xSemaphoreTake(xHbUARTRdTaskCheckSem, portMAX_DELAY) == pdTRUE) {
            gui32CheckHbUARTRdTask = 1;
            xSemaphoreGive(xHbUARTRdTaskCheckSem);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        if (xSemaphoreTake(xHbUARTRdTaskValSem, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if (gui32UARTRdTaskHb == 1)
            {
                heartbeat_count++;
                UARTSend("UART reader task alive\r\n");
            }
            else
            {
                gui32UARTRdTaskUnAliveCount++;
                if (gui32UARTRdTaskUnAliveCount > TASK_UNALIVE_CNT_UPPER_LIMIT)
                {
                    /* Sending heart beat failure case to BBG */
                    vLogData(LOG_LEVEL_CRITICAL, LOG_TYPE_HEARTBEAT, TASK_MAIN, UR_HB_FAILURE );
                    UARTSend("Report UART reader task's unalive state to BBG\r\n");
                }
            }
            xSemaphoreGive(xHbUARTRdTaskValSem);
        }

        if((heartbeat_count % 4) == 0)
        {
            /* Sending heart beat failure case to BBG */
            vLogData(LOG_LEVEL_INFO, LOG_TYPE_HEARTBEAT, TASK_MAIN, HB_SUCCESS );
        }
        heartbeat_count = 0;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


void vGetTime(char * input_string)
{
    time_t current_time;
    struct tm *info;
    time(&current_time);

    info = localtime(&current_time);
    strftime(input_string, 9,"%H:%M:%S",info);
}

void vPedometerTaskInit(void)
{
    uint8_t ctrl9_xl = 0;
    uint8_t ui8Msg[128];

    /* The following are sequence of configuration steps to access the step
     * counter
     * 1.   Set Zen_XL, Yen_XL and Xen_XL of CTRL9_XL (linear acceleration sensor
     *      control register - 0x18)
     *
     * 2.   The pedometer functions work at 26 Hz, so the accelerometer ODR
     *      must be set to a value of 26 Hz or higher
     *      (Accelerometer ODR configuration - CTRL_X1 reg (ODR_XL[3:0] = 0010))
     *
     * 3.   Enable the pedometer functions : To do this
     *      a.  Set FUNC_EN bit of the CTRL10_C register
     *      b.  Set PEDO_EN bit of the TAP_CFG register
     *
     * 4.   Configure step detector interrupt signal to INT1 interrupt pin
     *      To do this, set the INT1_STEP_DETECTOR bit of INT1_CTRL register
     *
     * 5.   Reset the step count to zero. To do this
     *      Set the PEDO_RST_STEP bit of the CTRL10_C register */

    /*  */

    /* Set Zen_XL, Yen_XL and Xen_XL of CTRL9_XL */
    vWritePedometerSensorRegister(PEDOMETER_SENSOR_CTRL9_XL_REG, PEDOMETER_SENSOR_LIN_ACCL_CFG_VAL);
    UARTSend("Finished configuring linear acceleration control.\r\n");

    SysCtlDelay(1000); //Delay by 2us

    /* Verify that the CTRL9_XL register is configured with the right value */
    ctrl9_xl = vReadPedometerSensorregister(PEDOMETER_SENSOR_CTRL9_XL_REG);
    memset(ui8Msg, '\0', sizeof(ui8Msg));
    sprintf(ui8Msg, "CTRL9_XL reg val after configuration. Configured Value: 0x%x, Expected: 0x%x\r\n",
           ctrl9_xl, PEDOMETER_SENSOR_LIN_ACCL_CFG_VAL);
    UARTSend(ui8Msg);

    /* Configure accelerometer ODR */
    vWritePedometerSensorRegister(PEDOMETER_SENSOR_CTRL1_X1_REG, PEDOMETER_SENSOR_ACC_ODR_CFG_VAL);
    UARTSend("Finished configuring accelerometer ODR to 26Hz.\r\n");

    /* Enable the pedometer functions */
    /* Set FUNC_EN bit of the CTRL10_C register */
    vWritePedometerSensorRegister(PEDOMETER_SENSOR_CTRL10_C_REG, PEDOMETER_SENSOR_FUNC_EN_CFG_VAL);
    UARTSend("Finished enabling FUNC_EN bit of CTRL10_C register.\r\n");

    /* Set PEDO_EN bit of the TAP_CFG register */
    vWritePedometerSensorRegister(PEDOMETER_SENSOR_TAP_CFG_REG, PEDOMETER_SENSOR_PEDO_EN_CFG_VAL);
    UARTSend("Finished enabling PEDO_EN bit of TAP_CFG register.\r\n");

    /* Set INT1_STEP_DETECTOR bit of INT1_CTRL register */
    vWritePedometerSensorRegister(PEDOMETER_SENSOR_INT_CTRL1_REG, PEDOMETER_SENSOR_INT1_STEP_DETECT_CFG_VAL);
    UARTSend("Finished enabling INT1_STEP_DETECTOR bit of INT_CTRL1 register.\r\n");

    /* Reset the step count to zero */
    vWritePedometerSensorRegister(PEDOMETER_SENSOR_CTRL10_C_REG, PEDOMETER_SENSOR_STEP_CNT_RST_CFG_VAL);
    UARTSend("Finished resetting step count.\r\n");

    gui32IsrCounter = 0;

    xPedometerDataAvail = xSemaphoreCreateBinary();
    xSemaphoreTake(xPedometerDataAvail, 0);

    /* Configure GPIO M (PM4) peripheral for INT1 interrupt that will be generated when a step is detected */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);

    /* Wait until the peripheral is ready */
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));

    MAP_GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_4);

    GPIOPadConfigSet(GPIO_PORTM_BASE, GPIO_PIN_4,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  // Enable weak pullup resistor for PF4

    GPIOIntDisable(GPIO_PORTM_BASE, GPIO_PIN_4);    // Disable interrupt for PM4
    GPIOIntClear(GPIO_PORTM_BASE, GPIO_PIN_4);      // Clear pending interrupts for PM4

    /* Register interrupt handler for GPIO pin PM4 */
    GPIOIntRegister(GPIO_PORTM_BASE, xPedometerStepCountIntHandler);

    /* Enable interrupt detection on rising edge */
    MAP_GPIOIntTypeSet(GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTM_BASE, GPIO_PIN_4);     // Enable interrupt for PM4
}

void vPedometerTask( void *pvParameters )
{
#if 0
    uint8_t status;
    static uint64_t ui64Count = 0;
    static uint64_t ui64ReadCounter = 0;

    uint8_t step_counter_l;
    uint8_t step_counter_h;
    uint16_t step_counter;
#endif

    uint8_t ui8_msg_str[128];

    /* Initialize the pedometer task */
    vPedometerTaskInit();

#if 0
    for (;;)
    {
        ui64Count++;

        /* Read the STATUS register(0x1E) */
        status = vReadPedometerSensorregister(PEDOMETER_SENSOR_STATUS_REG);

        if (status & 0x01) /* New accelerometer data available */
        {
            ui64ReadCounter++;

            /* Read the step_counter_l register(0x4B) */
            step_counter_l = vReadPedometerSensorregister(PEDOMETER_SENSOR_STEP_COUNTER_L_REG);
            //            UARTSend("step_counter_l is 0x%x\r\n", step_counter_l);

            /* Read the step_counter_h register(0x4C) */
            step_counter_h = vReadPedometerSensorregister(PEDOMETER_SENSOR_STEP_COUNTER_H_REG);
            //            UARTSend("step_counter_h is 0x%x\r\n", step_counter_h);

            step_counter = (step_counter_h << 8) | step_counter_l;

            //            if (ui64ReadCounter % 50 == 0)
            {
                memset(ui8_msg_str, '\0', sizeof(ui8_msg_str));
                sprintf(ui8_msg_str, "step_counter is 0x%x\r\n", step_counter);
                UARTSend(ui8_msg_str);

#if 0
                /* Wait on */
                if (xSemaphoreTake(xQueueMutex, portMAX_DELAY))
                {
                    sock_msg xSockMsg;
                    memset(&xSockMsg, '\0', sizeof(sock_msg));

                    //                    xSockMsg.eLogLevel = LOG_LEVEL_INFO;
                    //                    xSockMsg.eLogType = LOG_TYPE_DATA;
                    //                    xSockMsg.eSourceId = TASK_PEDOMETER;
                    strcpy(xSockMsg.data, ui8_msg_str);

                    if (xQueueSendToBack(xQueue, &xSockMsg, 10) != pdPASS)
                    {
                        UARTSend("Queue Send failed\r\n");
                    }
                    xSemaphoreGive(xQueueMutex);
                }
#else

                sock_msg xSockMsg;
                memset(&xSockMsg, '\0', sizeof(xSockMsg));

                xSockMsg.ui32LogLevel = LOG_LEVEL_STARTUP;
                xSockMsg.ui32LogType = LOG_TYPE_ERROR;
                xSockMsg.ui32SourceId = TASK_PEDOMETER;
                strcpy(xSockMsg.data, ui8_msg_str);
                uint32_t ui32SockMsgLen = sizeof(xSockMsg.ui32LogLevel) + sizeof(xSockMsg.ui32LogType) +
                        sizeof(xSockMsg.ui32SourceId) + strlen(xSockMsg.data);

                UARTSendToBBG((uint8_t *)&xSockMsg, ui32SockMsgLen);
#endif

                uint64_t ui64_idx = 0;
                for (ui64_idx = 0; ui64_idx < PEDOMETER_COUNT_PRINT_DELAY; ui64_idx++);
            }
        }
    }
#else

    for (;;)
    {
#ifdef UART_ENABLE
        if (gui32CheckHbPedTask == 1)
        {
            if (xSemaphoreTake(xHbPedTaskCheckSem, portMAX_DELAY) == pdTRUE) {
                gui32CheckHbPedTask = 0;
                xSemaphoreGive(xHbPedTaskCheckSem);
            }

            if (xSemaphoreTake(xHbPedTaskValSem, portMAX_DELAY) == pdTRUE) {
                gui32PedTaskHb = 1;
                xSemaphoreGive(xHbPedTaskValSem);
            }
        }
        else
        {
            if (xSemaphoreTake(xPedometerDataAvail, portMAX_DELAY))
            {
                  memset(ui8_msg_str, '\0', sizeof(ui8_msg_str));
#if 0
                  sprintf(ui8_msg_str, "Step counter is %d\r\n", gui32IsrCounter);
                  UARTSend(ui8_msg_str);
#endif
                  vLogData(LOG_LEVEL_CRITICAL, LOG_TYPE_DATA, TASK_PEDOMETER, gui32IsrCounter );
            }
        }
#else
        if (xSemaphoreTake(xPedometerDataAvail, portMAX_DELAY))
        {
            memset(ui8_msg_str, '\0', sizeof(ui8_msg_str));
#if 0
            sprintf(ui8_msg_str, "Step counter is %d\r\n", gui32IsrCounter);
            UARTSend(ui8_msg_str);
#endif

            vLogData(LOG_LEVEL_CRITICAL, LOG_TYPE_DATA, TASK_PEDOMETER, gui32IsrCounter );
        }
#endif
    }

#endif

}


void vLogData(uint32_t ui32LogLevel, uint32_t ui32LogType, uint32_t ui32SourceId, uint32_t ui32Data)
{
    sock_msg xSockMsg;
    memset(&xSockMsg, '\0', sizeof(xSockMsg));

    xSockMsg.ui32LogLevel = ui32LogLevel;
    xSockMsg.ui32LogType = ui32LogType;
    xSockMsg.ui32SourceId = ui32SourceId;
    xSockMsg.ui32Data = ui32Data;
    vGetTime(xSockMsg.ucTimeStamp);

    /* Push the data to be sent via UART to the queue */
    if (xSemaphoreTake(xQueueMutex, portMAX_DELAY))
    {
        if (xQueueSendToBack(xQueue, &xSockMsg, portMAX_DELAY) != pdPASS)
        {
            UARTSend("Queue Send failed\r\n");
        }
        xSemaphoreGive(xQueueMutex);
    }

}

int vTestPedometerSensor()
{
    int retVal = -1;

    uint32_t default_value = vReadPedometerSensorregister(WHO_AM_I_REG);
    if(default_value == 105)
    {
        UARTSend("Valid Pedometer Sensor\r\n");
        retVal = 0;
    }

    return retVal;
}

int vTestHumiditySensor()
{
    int retVal = -1;

    uint8_t default_value = read_humid_user_reg();
    if(default_value == 2)
    {
        UARTSend("Valid Humidity Sensor\r\n");
        retVal = 0;
    }

    return retVal;

}

void vWritePedometerSensorRegister(uint32_t ui32RegToWrite, uint32_t ui32RegVal)
{
    MAP_I2CMasterSlaveAddrSet(I2C_BASE, LSM6DS3_SENSOR_ADDR, false);
    MAP_I2CMasterDataPut(I2C_BASE, ui32RegToWrite);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(MAP_I2CMasterBusy(I2C_BASE));

    SysCtlDelay(500);

    MAP_I2CMasterDataPut(I2C_BASE, ui32RegVal);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(MAP_I2CMasterBusy(I2C_BASE));
}

uint32_t vReadPedometerSensorregister(uint32_t ui32RegToRead)
{
    uint32_t ui32RegVal = 0;
    int count=0;

    MAP_I2CMasterSlaveAddrSet(I2C_BASE, LSM6DS3_SENSOR_ADDR, false);
    MAP_I2CMasterDataPut(I2C_BASE, ui32RegToRead);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    for(count=0; count<100; count++)
    {
        SysCtlDelay(1000);
        if(!MAP_I2CMasterBusy(I2C_BASE)){
            break;
        }
    }

    count=0;
    SysCtlDelay(15000);

    MAP_I2CMasterSlaveAddrSet(I2C_BASE, LSM6DS3_SENSOR_ADDR, true);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    for(count=0; count<100; count++)
    {
        SysCtlDelay(1000);
        if(!MAP_I2CMasterBusy(I2C_BASE)){
            break;
        }
    }
    ui32RegVal = MAP_I2CMasterDataGet(I2C_BASE);

    return ui32RegVal;
}

uint8_t read_humid_user_reg()
{
    uint8_t reg_value;
    int count=0;

    /* Reading user register */
    MAP_I2CMasterSlaveAddrSet(I2C_BASE, HUMIDITY_SENSOR_ADDR, false);
    MAP_I2CMasterDataPut(I2C_BASE, READ_USER_REG);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    for(count=0; count<100; count++)
    {
        SysCtlDelay(1000);
        if(!MAP_I2CMasterBusy(I2C_BASE)){
            break;
        }
    }

    SysCtlDelay(1000);

    MAP_I2CMasterSlaveAddrSet(I2C_BASE, HUMIDITY_SENSOR_ADDR, true);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    for(count=0; count<100; count++)
    {
        SysCtlDelay(1000);
        if(!MAP_I2CMasterBusy(I2C_BASE)){
            break;
        }
    }
    SysCtlDelay(1000);
    reg_value = MAP_I2CMasterDataGet(I2C_BASE);
    return reg_value;
}

void xPedometerStepCountIntHandler(void)
{
    taskDISABLE_INTERRUPTS();
    MAP_GPIOIntDisable(GPIO_PORTM_BASE, GPIO_PIN_4);
    IntMasterDisable();
    MAP_GPIOIntClear(GPIO_PORTM_BASE, GPIO_PIN_4);  // Clear interrupt flag

    gui32IsrCounter++;

    xSemaphoreGive(xPedometerDataAvail);

    MAP_GPIOIntTypeSet(GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
    IntMasterEnable();
    MAP_GPIOIntEnable(GPIO_PORTM_BASE, GPIO_PIN_4);

    taskENABLE_INTERRUPTS();
}

void vHumidityTask(void *pvParameters)
{
    char buffer[25];

    gui32HumData = 0;

    while(1)
    {
        if (gui32CheckHbHumTask == 1)
        {
            if (xSemaphoreTake(xHbHumTaskCheckSem, portMAX_DELAY) == pdTRUE) {
                gui32CheckHbHumTask = 0;
                xSemaphoreGive(xHbHumTaskCheckSem);
            }

            if (xSemaphoreTake(xHbHumTaskValSem, portMAX_DELAY) == pdTRUE) {
                gui32HumTaskHb = 1;
                xSemaphoreGive(xHbHumTaskValSem);
            }
        }
        else
        {
            memset(buffer,'0',sizeof(buffer));
            float value = xReadHumidityValue();
#if 0
            sprintf(buffer,"Humidity Value:%3.2f\r\n",value);
            UARTSend(buffer);
#endif
            gui32HumData = (uint32_t)value;

            vLogData(LOG_LEVEL_INFO, LOG_TYPE_DATA, TASK_HUMIDITY, (uint32_t)value );

        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

float xReadHumidityValue(void)
{
    float fHumidValue;
    uint16_t ui16MsbVal = 0;
    uint16_t ui16LsbVal = 0;
    uint16_t ui16FinalVal = 0;

    /* Reading sensor data */
    MAP_I2CMasterSlaveAddrSet(I2C_BASE, HUMIDITY_SENSOR_ADDR, false);
    MAP_I2CMasterDataPut(I2C_BASE, TRIGGER_HUMIDITY_MEASUREMENT);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(MAP_I2CMasterBusy(I2C_BASE));

    vTaskDelay(pdMS_TO_TICKS(500));

    MAP_I2CMasterSlaveAddrSet(I2C_BASE, HUMIDITY_SENSOR_ADDR, true);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(MAP_I2CMasterBusy(I2C_BASE));
    vTaskDelay(pdMS_TO_TICKS(22));
    ui16MsbVal = MAP_I2CMasterDataGet(I2C_BASE);

    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(MAP_I2CMasterBusy(I2C_BASE));
    vTaskDelay(pdMS_TO_TICKS(22));
    ui16LsbVal = MAP_I2CMasterDataGet(I2C_BASE);

    ui16LsbVal &= 0xFC;

    ui16FinalVal = (ui16MsbVal << 8) | (ui16LsbVal);
    fHumidValue = (125.0*ui16FinalVal/65536)-6;
    return fHumidValue;
}


#ifdef UART_ENABLE

void vUARTWriterTask(void *pvParameters)
{

    sock_msg xSockMsg;

    for(;;)
    {
        if (gui32CheckHbUARTWrTask == 1)
        {
            if (xSemaphoreTake(xHbUARTWrTaskCheckSem, portMAX_DELAY) == pdTRUE) {
                gui32CheckHbUARTWrTask = 0;
                xSemaphoreGive(xHbUARTWrTaskCheckSem);
            }

            if (xSemaphoreTake(xHbUARTWrTaskValSem, portMAX_DELAY) == pdTRUE) {
                gui32UARTWrTaskHb = 1;
                xSemaphoreGive(xHbUARTWrTaskValSem);
            }
        }
        else
        {
            memset((void*)&xSockMsg,0,sizeof(xSockMsg));

            while(uxQueueSpacesAvailable(xQueue) != QUEUE_LENGTH)
            {
                if(xSemaphoreTake(xQueueMutex,portMAX_DELAY)){
                    if( xQueueReceive( xQueue, &xSockMsg, portMAX_DELAY ) != pdPASS ){
                        UARTSend("\ERROR:Queue Receive\r\n");
                    }
                    else{
                        UARTSendToBBG((uint8_t *)&xSockMsg, 25);
                    }
                    xSemaphoreGive(xQueueMutex);
                }
            }
        }
    }
}

void vUARTReaderTask(void *pvParameters)
{
    uint32_t size = 0;
    uint8_t buffer[20];
    int i = 0;
    char ui8CharToSend;

    for (;;)
    {

        if (gui32CheckHbUARTRdTask == 1)
        {
            if (xSemaphoreTake(xHbUARTRdTaskCheckSem, portMAX_DELAY) == pdTRUE) {
                gui32CheckHbUARTRdTask = 0;
                xSemaphoreGive(xHbUARTRdTaskCheckSem);
            }

            if (xSemaphoreTake(xHbUARTRdTaskValSem, portMAX_DELAY) == pdTRUE) {
                //gui32UARTRdTaskHb = 1;
                xSemaphoreGive(xHbUARTRdTaskValSem);
            }
        }
        else
        {
            if(UARTCharsAvail(UART7_BASE))
            {
                memset(buffer, 0, sizeof(buffer));
                size = 20;
                i = 0;

                while(size--)
                {
                    /* Read the next character from the UART and write it back to the UART. */
                    sprintf(&ui8CharToSend, "%c", (char)UARTCharGet(UART7_BASE));
                    buffer[i++] = ui8CharToSend;
                }

                uint8_t printMsg[32];
                memset(printMsg, '\0', sizeof(printMsg));
                sprintf(printMsg, "Req Recipient: %d\r\n", ((bbg_req_msg *)buffer)->req_recipient);
                UARTSend(printMsg);

                memset(printMsg, '\0', sizeof(printMsg));
                sprintf(printMsg, "Params: %d\r\n", ((bbg_req_msg *)buffer)->params);
                UARTSend(printMsg);

                memset(printMsg, '\0', sizeof(printMsg));
                sprintf(printMsg, "Req Msg: %s\r\n", ((bbg_req_msg *)buffer)->rq_msg);
                UARTSend(printMsg);

                if(strncmp(((bbg_req_msg *)buffer)->rq_msg, "get_ped_data",MESSAGE_LENGTH) == 0)
                {
                    if (((bbg_req_msg *)buffer)->req_recipient == TASK_PEDOMETER)
                    {
                        vLogData(LOG_LEVEL_INFO, LOG_TYPE_DATA, TASK_PEDOMETER, gui32IsrCounter );
                    }
                }
                else if(strncmp(((bbg_req_msg *)buffer)->rq_msg, "get_ped_ssid",MESSAGE_LENGTH) == 0)
                {
                    if(((bbg_req_msg *)buffer)->req_recipient == TASK_PEDOMETER)
                    {
                        vLogData(LOG_LEVEL_INFO, LOG_TYPE_DATA, TASK_PEDOMETER, vReadPedometerSensorregister(WHO_AM_I_REG) );
                    }
                }
                else if(strncmp(((bbg_req_msg *)buffer)->rq_msg, "get_hum_data",MESSAGE_LENGTH) == 0)
                {
                    if(((bbg_req_msg *)buffer)->req_recipient == TASK_HUMIDITY)
                    {
                        vLogData(LOG_LEVEL_INFO, LOG_TYPE_DATA, TASK_HUMIDITY, gui32HumData );
                    }
                }
                else if(strncmp(((bbg_req_msg *)buffer)->rq_msg, "get_hum_usid",MESSAGE_LENGTH) == 0)
                {
                    if(((bbg_req_msg *)buffer)->req_recipient == TASK_HUMIDITY)
                    {
                        vLogData(LOG_LEVEL_INFO, LOG_TYPE_DATA, TASK_HUMIDITY, read_humid_user_reg() );
                    }
                }

                vTaskDelay(pdMS_TO_TICKS(500));

            }
        }

    }

}

void UART7Init(void)
{
    // Enable the peripherals.
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    // Set GPIO PB0 and PB1 as UART pins.
    GPIOPinConfigure(GPIO_PC5_U7TX);
    GPIOPinConfigure(GPIO_PC4_U7RX);
    MAP_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    // Configure the UART for 115200 baud, 8-N-1 operation.
    MAP_UARTConfigSetExpClk(UART7_BASE, 120000000, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                    UART_CONFIG_PAR_NONE));

    xUARTToBBGSemaphore = xSemaphoreCreateMutex();
}

#endif

void UARTSendToBBG(char *pucBuffer, uint32_t ui32BufLen)
{
    if (pucBuffer != NULL)
    {
        if( xSemaphoreTake( xUARTToBBGSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
            while(ui32BufLen)
            {
                UARTCharPut(UART7_BASE, *pucBuffer);
                pucBuffer++;
                ui32BufLen--;
            }
            xSemaphoreGive( xUARTToBBGSemaphore );
        }
    }
}

void UARTIntHandler(void)
{
    uint32_t ui32Status;

    /* Get the interrupt status. */
    ui32Status = MAP_UARTIntStatus(UART7_BASE, true);

    /* Clear the asserted interrupts. */
    MAP_UARTIntClear(UART7_BASE, ui32Status);

    /* Loop while there are characters in the receive FIFO. */
    while(MAP_UARTCharsAvail(UART7_BASE))
    {
        /* Read the next character from the UART and write it back to the UART. */
        uint8_t ui8CharToSend;
        sprintf(&ui8CharToSend, "%c", UARTCharGet(UART7_BASE));
        UARTSend(&ui8CharToSend);
    }
}
