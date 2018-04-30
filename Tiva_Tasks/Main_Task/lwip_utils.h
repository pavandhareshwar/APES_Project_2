/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/122/2018
* File:         lwip_utils.h
* Description:  Header file containing the macros, structs/enums, globals
                and function prototypes for source file lwip_utils.c
*************************************************************************/

#ifndef LWIP_UTILS_H_
#define LWIP_UTILS_H_


/*---------------------------------- INCLUDES -------------------------------*/

#include "inc/hw_ints.h"
#include "inc/hw_types.h"

#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#include "utils/lwiplib.h"
#include "utils/locator.h"
#include "utils/ustdlib.h"

#include "uart_utils.h"

#include "FreeRTOS.h"

#include "semphr.h"

/*---------------------------------- GLOBALS --------------------------------*/

#define SYSTICK_INT_PRIORITY        0x80
#define ETHERNET_INT_PRIORITY       0xC0

#define LOCAL_SOCKET_PORT           61121
#define REMOTE_SOCKET_PORT          61122
#define SOCKET_POLL_INTERVAL        4

#define SOCKET_TCP_PRIO             TCP_PRIO_MIN

/* Forward Mention of IP addresses */
#define BBG_IP_ADDRESS              167772260  // BBG ip address 10.0.0.100
#define TIVA_IP_ADDRESS             167772280  // Tiva ip address 10.0.0.120
#define TIVA_NETMASK                4294967040    // Tiva Netmask 255.255.255.0
#define TIVA_GATEWAY_ADDR           167772161    // Tiva Gateway Addr 10.0.0.1

#define CLIENT_MODE                 1
//#define SERVER_MODE                 1

uint32_t gui32IpAddr;
uint32_t gui32IpAddrCreated;
bool gbConnectionAvailable;
bool gbIsReady;
bool gbReadyToSend;

uint32_t g_ui32SysClock;

struct tcp_pcb *pcb;

SemaphoreHandle_t xSemLwIP;

/*---------------------------- FUNCTION PROTOTYPES --------------------------*/
void lwip_socket_init_prep(void);

void lwip_socket_init(void);

void DisplayIPAddress(uint32_t ui32Addr);

static err_t socket_close_conn(struct tcp_pcb *pcb);
static err_t socket_poll(void *arg, struct tcp_pcb *pcb);
static err_t socket_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t socket_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
static void socket_err(void *arg, err_t err);
static err_t socket_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t socket_connCallBack(void *arg, struct tcp_pcb *tpcb, err_t err);

void lwIPHostTimerHandler(void);
void lwIPSysTickHandler(void);

#endif /* LWIP_UTILS_H_ */
