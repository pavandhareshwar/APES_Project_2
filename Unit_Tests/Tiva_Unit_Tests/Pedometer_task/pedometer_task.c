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
#include "pedometer_task.h"

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

//initialize I2C module 0
//Slightly modified version of TI's example code
void InitI2C0(void)
{
    //enable I2C module 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    //reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    //enable GPIO peripheral that contains I2C 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

}

void i2c_write(uint32_t ui32Base, uint8_t ui8Data){

    I2CMasterDataPut(ui32Base,ui8Data);
    //Initiate send of data from the MCU
    I2CMasterControl(ui32Base, I2C_MASTER_CMD_SINGLE_SEND);
    // Wait until MCU is done transferring.
    while(I2CMasterBusy(ui32Base));
}

/**
​* ​ ​ @brief​ : Task-1 definition
​* ​ ​
​*
​* ​ ​ @param​ ​ pvParameters Parameters for task1
​*
​* ​ ​ @return​ ​None
​*/
void pedometer_task( void *pvParameters )
{

    /* Intializing I2C */
    //InitI2C0();

    /* Setting the slave address */
    /*I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDR, false);
    /* Writing the fucntion configuration reg
    i2c_write(I2C0_BASE, FUNC_CFG_ACCESS_REG);
    i2c_write(I2C0_BASE, FUNC_CFG_ACCESS_REG_VALUE);

    /* Reading step count low
    /* Setting the slave address
    I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDR, false);
    i2c_write(I2C0_BASE, STEP_COUNT_L_REG);
    I2CMasterSlaveAddrSet(I2C0_BASE, SLAVE_ADDR, false);
    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));

    uint32_t data = I2CMasterDataGet(I2C0_BASE);

    UARTprintf("Value:%d\n",data);*/
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

    UARTprintf("Start of Pedometer task\n");

    /* Creating task-3 */
    if(xTaskCreate(pedometer_task, "Pedometer_Task", TASK_BUFFER, NULL, 1, NULL) != pdFALSE){

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
