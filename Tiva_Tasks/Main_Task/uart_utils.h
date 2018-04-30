/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/122/2018
* File:         uart_utils.h
* Description:  Header file containing the macros, structs/enums, globals
                and function prototypes for source file uart_utils.c
*************************************************************************/

#ifndef UART_UTILS_H_
#define UART_UTILS_H_

/*---------------------------------- INCLUDES -------------------------------*/
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "inc/hw_memmap.h"

/*---------------------------------- GLOBALS --------------------------------*/

extern uint32_t g_ui32SysClock;

/*---------------------------- FUNCTION PROTOTYPES --------------------------*/


/**
 *  @brief UART Init
 *
 *  This function will perform the system level initialization of UART0
 *
 *  @param      void
 *
 *  @return     void
 */
void UART0_Init(void);

/**
 *  @brief UART Send function
 *
 *  This function will send the data in @param pui8MsgStr to UART
 *
 *  @param      pui8MsgStr      : pointer to message
 *
 *  @return     void
 */
void UARTSend(char *pui8MsgStr);


#endif /* UART_UTILS_H_ */
