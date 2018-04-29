/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/23/2018
* File:         external_app.h
* Description:  Header file containing the macros, structs/enums, globals
                and function prototypes for source file external_app.c
*************************************************************************/

#ifndef _EXTERNAL_APP_H_
#define _EXTERNAL_APP_H_

/*---------------------------------- INCLUDES -------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

/*---------------------------------- INCLUDES -------------------------------*/

/*----------------------------------- MACROS --------------------------------*/
#define SERVER_PORT_NUM 			         8500
#define SERVER_LISTEN_QUEUE_SIZE	         5

#define BUFF_SIZE			                 1024

#define SOCK_REQ_MSG_API_MSG_LEN             13

#define REQ_RECP_PEDOMETER_TASK              0x1 
#define REQ_RECP_HUMIDITY_TASK               0x2

/*----------------------------------- MACROS --------------------------------*/

/*---------------------------------- GLOBALS --------------------------------*/

/*---------------------------------- GLOBALS --------------------------------*/

/*---------------------------- STRUCTURES/ENUMERATIONS ----------------------*/
struct _ext_app_req_msg_struct_
{
    int req_recipient;
    int params;
    char req_api_msg[SOCK_REQ_MSG_API_MSG_LEN];
};

/*---------------------------- STRUCTURES/ENUMERATIONS ----------------------*/

/*---------------------------- FUNCTION PROTOTYPES --------------------------*/

/*---------------------------- FUNCTION PROTOTYPES --------------------------*/

#endif
