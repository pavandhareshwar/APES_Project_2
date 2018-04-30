/*******************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/22/2018
* File:         uart_utils.c
* Description:  Source file containing the functionality and implementation
*               of the uart.
********************************************************************************/

/* Headers Section */
#include "uart_utils.h"

void UART0_Init(void)
{
    /* Enable the GPIO Peripheral used by the UART. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Enable UART0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure GPIO Pins for UART mode. */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART : baud rate: 115200, 1 stop bit and no parity */
    MAP_UARTConfigSetExpClk(UART0_BASE, g_ui32SysClock, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                        UART_CONFIG_PAR_NONE));
}

void UARTSend(char *pui8MsgStr)
{
    uint32_t ui32_str_len = 0;

    if(pui8MsgStr != NULL)
    {
        ui32_str_len = strlen(pui8MsgStr);

        while (ui32_str_len != 0)
        {
            /* Write a single character to UART */
            UARTCharPut(UART0_BASE, *pui8MsgStr);
            ui32_str_len--;
            pui8MsgStr++;
        }

    }
}
