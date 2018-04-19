/* Headers Section */
#include "pedometer_task.h"

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

    /* Initializing the UART */
    UART0_Init();

    I2C0_Init();

    QueueCreate();

    if (CreateTasks() == pdPASS)
    {
        /* Start the scheduler */
        vTaskStartScheduler();

        for(;;);
    }
    else
    {
        UARTSend("Task creation failed\r\n");
        i32RetVal = -1;
    }

	return i32RetVal;
}

void UART0_Init(void)
{
    /* Enable the GPIO Peripheral used by the UART. */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Enable UART0 */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure GPIO Pins for UART mode. */
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART : baud rate: 115200, 1 stop bit and no parity */
    ROM_UARTConfigSetExpClk(UART0_BASE, g_ui32SysClock, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                        UART_CONFIG_PAR_NONE));

    /* Create a semaphore for UART */
    xUARTSemaphore = xSemaphoreCreateMutex();

    UARTSend("UART0 initialization done\r\n");
}

void I2C0_Init(void)
{
    /* Enable the GPIO Peripheral used by the I2C0 */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    /* Configure GPIO pins for I2C0 */
    ROM_GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    ROM_GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    /* Configure GPIO pin PB2 as SDA and PB3 as SCL */
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    ROM_GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    /* Disable-reset-enable the I2C0 peripheral */
    ROM_SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C0);
    ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    /* Configure master module */
    ROM_I2CMasterInitExpClk(I2C_BASE, SYSTEM_CLOCK, true);
}

int QueueCreate(void)
{
    xQueue = xQueueCreate(QUEUE_LENGTH, sizeof(sock_msg));
    if (xQueue != NULL)
    {
        UARTSend("Queue created successfully\r\n");

        xQueueMutex = xSemaphoreCreateMutex();
        return 0;
    }
    else
    {
        UARTSend("Queue creation failed\r\n");
        return -1;
    }
}

BaseType_t CreateTasks(void)
{
    BaseType_t xRetVal = pdTRUE;

    if(xTaskCreate(vPedometerTask, "Pedometer_Task", TASK_BUFFER, NULL, 1, NULL) == pdFALSE)
    {
        UARTSend("Pedometer Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }

    if(xTaskCreate(vUARTTask, "UART_Task", TASK_BUFFER, NULL, 1, NULL) == pdFALSE)
    {
        UARTSend("UART Task creation failed\r\n");
        xRetVal = pdFAIL;
        return xRetVal;
    }

    return xRetVal;
}

void UARTSend(uint8_t *pui8MsgStr)
{
    uint32_t ui32_str_len = 0;

    if(pui8MsgStr != NULL)
    {
        ui32_str_len = strlen(pui8MsgStr);

//        if( xSemaphoreTake( xUARTSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
            while (ui32_str_len != 0)
            {
                /* Write a single character to UART */
                UARTCharPut(UART0_BASE, *pui8MsgStr);
                ui32_str_len--;
                pui8MsgStr++;
            }

//            xSemaphoreGive( xUARTSemaphore );
        }
    }
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
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);

    /* Wait until the peripheral is ready */
    while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));

    ROM_GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_4);

    GPIOPadConfigSet(GPIO_PORTM_BASE, GPIO_PIN_4,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  // Enable weak pullup resistor for PF4

    GPIOIntDisable(GPIO_PORTM_BASE, GPIO_PIN_4);    // Disable interrupt for PM4
    GPIOIntClear(GPIO_PORTM_BASE, GPIO_PIN_4);      // Clear pending interrupts for PM4

    /* Register interrupt handler for GPIO pin PM4 */
    GPIOIntRegister(GPIO_PORTM_BASE, xPedometerStepCountIntHandler);

    /* Enable interrupt detection on rising edge */
    ROM_GPIOIntTypeSet(GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
    ROM_GPIOIntEnable(GPIO_PORTM_BASE, GPIO_PIN_4);     // Enable interrupt for PM4
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
    sock_msg xSockMsg;

    for (;;)
    {
        memset(&xSockMsg, '\0', sizeof(xSockMsg));
        if (xSemaphoreTake(xPedometerDataAvail, portMAX_DELAY))
        {
            memset(ui8_msg_str, '\0', sizeof(ui8_msg_str));
//            sprintf(ui8_msg_str, "step counter is %d\r\n", gui32IsrCounter);
            UARTSend(ui8_msg_str);

            xSockMsg.ui32LogLevel = LOG_LEVEL_INFO;
            xSockMsg.ui32LogType = LOG_TYPE_DATA;
            xSockMsg.ui32SourceId = TASK_PEDOMETER;
            xSockMsg.ui32StepCount = gui32IsrCounter;

            /* Push the data to be sent via UART to the queue */
            if (xSemaphoreTake(xQueueMutex, portMAX_DELAY))
            {
                if (xQueueSendToBack(xQueue, &xSockMsg, 10) != pdPASS)
                {
                    UARTSend("Queue Send failed\r\n");
                }
                xSemaphoreGive(xQueueMutex);
            }
        }
    }

#endif

}

void vWritePedometerSensorRegister(uint32_t ui32RegToWrite, uint32_t ui32RegVal)
{
    ROM_I2CMasterSlaveAddrSet(I2C_BASE, LSM6DS3_SENSOR_ADDR, false);
    ROM_I2CMasterDataPut(I2C_BASE, ui32RegToWrite);
    ROM_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(ROM_I2CMasterBusy(I2C_BASE));

    SysCtlDelay(500); //Delay by 1us

    ROM_I2CMasterDataPut(I2C_BASE, ui32RegVal);
    ROM_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(ROM_I2CMasterBusy(I2C_BASE));
}

uint32_t vReadPedometerSensorregister(uint32_t ui32RegToRead)
{
    uint32_t ui32RegVal = 0;

    ROM_I2CMasterSlaveAddrSet(I2C_BASE, LSM6DS3_SENSOR_ADDR, false);
    ROM_I2CMasterDataPut(I2C_BASE, ui32RegToRead);
    ROM_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(ROM_I2CMasterBusy(I2C_BASE));

    SysCtlDelay(100); //Delay by 1us

    ROM_I2CMasterSlaveAddrSet(I2C_BASE, LSM6DS3_SENSOR_ADDR, true);
    ROM_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(ROM_I2CMasterBusy(I2C_BASE));
    ui32RegVal = ROM_I2CMasterDataGet(I2C_BASE);

    return ui32RegVal;
}

void xPedometerStepCountIntHandler(void)
{
    taskDISABLE_INTERRUPTS();
    ROM_GPIOIntDisable(GPIO_PORTM_BASE, GPIO_PIN_4);
    IntMasterDisable();
    ROM_GPIOIntClear(GPIO_PORTM_BASE, GPIO_PIN_4);  // Clear interrupt flag

    gui32IsrCounter++;

    xSemaphoreGive(xPedometerDataAvail);

    ROM_GPIOIntTypeSet(GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
    IntMasterEnable();
    ROM_GPIOIntEnable(GPIO_PORTM_BASE, GPIO_PIN_4);

    taskENABLE_INTERRUPTS();
}

void vUARTTask(void *pvParameters)
{
    UART7Init();
    sock_msg xSockMsg;

    for(;;)
    {
        memset((void*)&xSockMsg,0,sizeof(xSockMsg));

        while(uxQueueSpacesAvailable(xQueue) != QUEUE_LENGTH){
            if(xSemaphoreTake(xQueueMutex,portMAX_DELAY)){
                if( xQueueReceive( xQueue, &xSockMsg, portMAX_DELAY ) != pdPASS ){
                    UARTSend("\ERROR:Queue Receive\r\n");
                }
                else{
                    UARTSendToBBG((uint8_t *)&xSockMsg, sizeof(xSockMsg));
                }
                xSemaphoreGive(xQueueMutex);
            }
        }
    }

}

void UART7Init(void)
{
    // Enable the peripherals.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    // Set GPIO PB0 and PB1 as UART pins.
    GPIOPinConfigure(GPIO_PC5_U7TX);
    GPIOPinConfigure(GPIO_PC4_U7RX);
    ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    // Configure the UART for 115200 baud, 8-N-1 operation.
    ROM_UARTConfigSetExpClk(UART7_BASE, 120000000, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                    UART_CONFIG_PAR_NONE));

//    ROM_IntEnable(INT_UART7);
//    ROM_UARTIntEnable(UART7_BASE, UART_INT_RX | UART_INT_RT);

//    UARTLoopbackEnable(UART7_BASE);
}

void UARTSendToBBG(char *pucBuffer, uint32_t ui32BufLen)
{
    if (pucBuffer != NULL)
    {
        while(ui32BufLen)
        {
//            UARTCharPut(UART7_BASE, *pucBuffer);
            ROM_UARTCharPutNonBlocking(UART7_BASE, *pucBuffer);
            pucBuffer++;
            ui32BufLen--;
        }
    }
}

void UARTIntHandler(void)
{
    uint32_t ui32Status;

    /* Get the interrupt status. */
    ui32Status = ROM_UARTIntStatus(UART7_BASE, true);

    /* Clear the asserted interrupts. */
    ROM_UARTIntClear(UART7_BASE, ui32Status);

    /* Loop while there are characters in the receive FIFO. */
    while(ROM_UARTCharsAvail(UART7_BASE))
    {
        /* Read the next character from the UART and write it back to the UART. */
//        UARTprintf(ROM_UARTCharGetNonBlocking(UART7_BASE));
        uint8_t ui8CharToSend;
        sprintf(&ui8CharToSend, "%c", ROM_UARTCharGetNonBlocking(UART7_BASE));
        UARTSend(&ui8CharToSend);
    }
}
