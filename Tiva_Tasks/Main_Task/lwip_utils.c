/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/122/2018
* File:         lwip_utils.h
* Description:  Source file containing the functionality and implementation
*               of the LWIP
*
* Note: References for this code is taken from Tivaware library,
*       TI forums and demo code/examples from FreeRTOS LWIP source code .
* Links: http://e2e.ti.com/support/microcontrollers/tiva_arm/f/908/p/409862/1478922
*************************************************************************/

/* Headers Section */
#include "lwip_utils.h"

void lwip_socket_init_prep(void)
{
    uint8_t pui8MACArray[8];

    uint32_t ui32User0, ui32User1;

    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        // Let the user know there is no MAC address
        //
        UARTSend("No MAC programmed!\r\n");
        while(1){ }
    }

    /*  Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
        address needed to program the hardware registers, then program the MAC
        address into the Ethernet Controller registers.
    */
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    lwIPInit(g_ui32SysClock, pui8MACArray, TIVA_IP_ADDRESS, TIVA_NETMASK,
             TIVA_GATEWAY_ADDR, IPADDR_USE_STATIC);

    LocatorInit();
    LocatorMACAddrSet(pui8MACArray);
    LocatorAppTitleSet("EK-TM4C1294XL enet_io");

    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

}

void lwip_socket_init(void)
{
//    const ip_addr_t ip_addr_bbg = { (u32_t)BBG_IP_ADDRESS };

    UARTSend("Initializing listener\r\n");
    struct ip_addr bbg_ip_addr;

    IP4_ADDR(&bbg_ip_addr, 10, 0, 0, 100);

    lwip_socket_init_prep();

    pcb = tcp_new();
    if (pcb == NULL)
    {
        UARTSend("tcp_new failed\r\n");
    }

    while(!gui32IpAddrCreated);

#ifdef CLIENT_MODE

    tcp_err(pcb, socket_err);

    tcp_bind(pcb, IP_ADDR_ANY, LOCAL_SOCKET_PORT);

    /* If connect was successful, "socket_connCallBack" function will be called */
    if(tcp_connect(pcb, (ip_addr_t *)&bbg_ip_addr, REMOTE_SOCKET_PORT, socket_connCallBack) == ERR_OK)
    {
        UARTSend("Client connected\r\n");
        /* Specify the callback function that is to be called when data has been
         * successfully received by the host */
        tcp_sent(pcb, socket_sent);

        /* Specify the callback function that will be called when new data arrives */
        tcp_recv(pcb, socket_recv);

        gbConnectionAvailable = true;
    }
    else
    {
        UARTSend("Failed to Connect as client\r\n");
    }
#endif

#ifdef SERVER_MODE
    uint8_t ui8MsgBuffer[64];

    memset(ui8MsgBuffer, '\0', sizeof(ui8MsgBuffer));

    tcp_setprio(pcb, SOCKET_TCP_PRIO);
    /* set SOF_REUSEADDR here to explicitly bind httpd to multiple interfaces */
    err_t err = tcp_bind(pcb, IP_ADDR_ANY, LOCAL_SOCKET_PORT);
    if ( err != ERR_OK )
    {
        sprintf(ui8MsgBuffer, "tcp_bind failed with error %d\r\n", err);
        UARTSend(ui8MsgBuffer);
    }
    sprintf(ui8MsgBuffer, "Bound to port %d\r\n", LOCAL_SOCKET_PORT);
    UARTSend(ui8MsgBuffer);

    pcb = tcp_listen(pcb);
    if (pcb == NULL)
    {
        UARTSend("tcp_listen failed\r\n");
    }

    /* initialize callback arg and accept callback */
    tcp_arg(pcb, pcb);
    tcp_accept(pcb, socket_accept);
#endif
}

void SysTickIntHandler(void)
{
    //
    // Call the lwIP timer handler.
    //
    lwIPTimer(10);
}

void lwIPHostTimerHandler(void)
{
    uint32_t ui32Idx, ui32NewIPAddress;

    // Get the current IP address.
    ui32NewIPAddress = lwIPLocalIPAddrGet();

    // See if the IP address has changed.
    if(ui32NewIPAddress != gui32IpAddr)
    {
        // See if there is an IP address assigned.
        if(ui32NewIPAddress == 0xffffffff)
        {
            // Indicate that there is no link.
            UARTSend("Waiting for link.\r\n");
        }
        else if(ui32NewIPAddress == 0)
        {
            //
            // There is no IP address, so indicate that the DHCP process is
            // running.
            //
            UARTSend("Waiting for IP address.\r\n");
        }
        else
        {
            //
            // Display the new IP address.
            //
            gui32IpAddrCreated = true;   /*IP getting created*/
            DisplayIPAddress(ui32NewIPAddress);
            gbIsReady = true;
        }

        //
        // Save the new IP address.
        //
        gui32IpAddr = ui32NewIPAddress;
    }
}

static err_t socket_connCallBack(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    UARTSend("Connected as client\r\n");
    gbReadyToSend = true;

    return ERR_OK;
}

/* We'll use this when Tiva is used as a server and has to accept
 * connection request from BBG. This function will serve as a
 * callback that will be called when a new connection arrives
 * for a listening TCP PCB */

static err_t socket_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    char ui8MsgBuffer[64];

    memset(ui8MsgBuffer, '\0', sizeof(ui8MsgBuffer));

    sprintf(ui8MsgBuffer, "Connected %p / %p\r\n", (void*)pcb, arg);
    UARTSend(ui8MsgBuffer);
     struct tcp_pcb_listen *lpcb = (struct tcp_pcb_listen*)arg;

    /* Decrease the listen backlog counter */
    tcp_accepted(lpcb);

    /* Set priority */
    tcp_setprio(pcb, SOCKET_TCP_PRIO);

    /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
    tcp_arg(pcb, pcb);

    /* Set up the various callback functions */
    tcp_recv(pcb, socket_recv);
    tcp_err(pcb, socket_err);
    tcp_poll(pcb, socket_close_conn, SOCKET_POLL_INTERVAL);
    tcp_sent(pcb, socket_sent);
    tcp_write(pcb, "Hello\0", 6, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);

    return ERR_OK;
}

static void socket_err(void *arg, err_t err)
{
    char ui8MsgBuffer[64];

    memset(ui8MsgBuffer, '\0', sizeof(ui8MsgBuffer));
    sprintf(ui8MsgBuffer, "tcp_err: %s\r\n", lwip_strerr(err));

    UARTSend(ui8MsgBuffer);
    gbConnectionAvailable = false;
    socket_close_conn(pcb);
}

static err_t socket_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    UARTSend("sent data\r\n");
    return (err_t) 0;
}


static err_t socket_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    char ui8MsgBuffer[64];

    memset(ui8MsgBuffer, '\0', sizeof(ui8MsgBuffer));
    sprintf(ui8MsgBuffer, "Received data: %s\r\n", (char *)p->payload);

    UARTSend(ui8MsgBuffer);
    return ERR_OK;
}


static err_t socket_poll(void *arg, struct tcp_pcb *pcb)
{
    return (err_t) 0;
}


static err_t socket_close_conn(struct tcp_pcb *pcb)
{
    char ui8MsgBuffer[64];


    memset(ui8MsgBuffer, '\0', sizeof(ui8MsgBuffer));


    err_t err;
    UARTSend("Closing connection\r\n");

    /* Set the callbacks to none */
    tcp_arg(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_poll(pcb, NULL, 0);
    tcp_sent(pcb, NULL);

    err = tcp_close(pcb);
    if (err != ERR_OK) {
        sprintf(ui8MsgBuffer, "Error %d closing connection", err);
        UARTSend(ui8MsgBuffer);
    }
    return err;
}

void DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    // Convert the IP Address into a string.
    sprintf(pcBuf, "%d.%d.%d.%d\r\n", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    // Display the string.
    UARTSend("IP Addr is: ");
    UARTSend(pcBuf);
}
