/*************************************************************************************
*
* FileName        :    problem_3.c
* Description     :    This file is example for creating tasks and example
*                       of using queue and event groups in TIVA board
*                       using FreeRTOS.
*
* File Author Name:    Sridhar Pavithrapu
* References      :    None
*
***************************************************************************************/

/* Headers Section */
#include <humidity_task.h>

/**
​* ​ ​ @brief​ : Initialize the UART
​* ​ ​
​*
​* ​ ​ @param​ ​ None
​*
​* ​ ​ @return​ ​None
​*/
void UART0_Initialize(){

    /* Enable the GPIO Peripheral used by the UART. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Enable UART0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure GPIO Pins for UART mode. */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Initialize the UART for console I/O. */
    UARTStdioConfig(0, 115200, g_ui32SysClock);
}

void write_humid_user_reg(uint8_t reg_value)
{
    /* Writing user register */
    MAP_I2CMasterSlaveAddrSet(I2C_BASE, HUMIDITY_SENSOR_ADDR, false);
    MAP_I2CMasterDataPut(I2C_BASE, WRITE_USER_REG);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(MAP_I2CMasterBusy(I2C_BASE));

    vTaskDelay(pdMS_TO_TICKS(500));

    MAP_I2CMasterDataPut(I2C_BASE, reg_value);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(MAP_I2CMasterBusy(I2C_BASE));
}

uint8_t read_humid_user_reg()
{
    uint8_t reg_value;
    /* Reading user register */
    MAP_I2CMasterSlaveAddrSet(I2C_BASE, HUMIDITY_SENSOR_ADDR, false);
    MAP_I2CMasterDataPut(I2C_BASE, READ_USER_REG);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(MAP_I2CMasterBusy(I2C_BASE));

    vTaskDelay(pdMS_TO_TICKS(500));

    MAP_I2CMasterSlaveAddrSet(I2C_BASE, HUMIDITY_SENSOR_ADDR, true);
    MAP_I2CMasterControl(I2C_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(MAP_I2CMasterBusy(I2C_BASE));
    reg_value = MAP_I2CMasterDataGet(I2C_BASE);
    return reg_value;
}

float read_humid_value()
{
    float humid_value;
    uint16_t ui16MsbVal = 0;
    uint16_t ui16LsbVal = 0;
    uint16_t ui16FinalVal = 0;

    /* Readding sensor data */
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
    humid_value = (125.0*ui16FinalVal/65536)-6;
    return humid_value;
}

/**
​* ​ ​ @brief​ : Task-1 definition
​* ​ ​
​*
​* ​ ​ @param​ ​ pvParameters Parameters for task1
​*
​* ​ ​ @return​ ​None
​*/
void humidity_task( void *pvParameters )
{
    char buffer[25];
    while(1)
    {
        memset(buffer,'0',sizeof(buffer));
        float value = read_humid_value();
        sprintf(buffer,"Humid Value:%f\n",value);
        UARTprintf(buffer);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

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

/**
 * main.c
 */
int main(void)
{
    /* Set the clocking to run directly from the crystal at 120MHz. */
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                                 SYSCTL_OSC_MAIN |
                                                 SYSCTL_USE_PLL |
                                                 SYSCTL_CFG_VCO_480), 120000000);

    /* Initializing the UART */
    UART0_Initialize();

    /* Initialize I2C */
    I2C0_Init();

    UARTprintf("Start of humidity task\n");

    /* Creating task-3 */
    if(xTaskCreate(humidity_task, "Humidity_Task", TASK_BUFFER, NULL, 1, NULL) != pdFALSE){

        /* Starting the scheduler */
        vTaskStartScheduler();

        /* If all is well, the scheduler will now be running, and the following
        line will never be reached.  If the following line does execute, then
        there was insufficient FreeRTOS heap memory available for the idle and/or
        timer tasks to be created.  See the memory management section on the
        FreeRTOS web site for more details. */
        for(;;);

    }
	return 0;
}
