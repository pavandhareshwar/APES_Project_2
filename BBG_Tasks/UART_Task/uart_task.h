/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/08/2018
* File:         uart_task.h
* Description:  Header file containing the macros, structs/enums, globals
                and function prototypes for source file uart_task.c
*************************************************************************/

#ifndef _UART_TASK_H_
#define _UART_TASK_H_

/*---------------------------------- INCLUDES -------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>

#include <mqueue.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <termios.h>

/*---------------------------------- INCLUDES -------------------------------*/

/*----------------------------------- MACROS --------------------------------*/
// Message queue attribute macros 
#define MSG_QUEUE_MAX_NUM_MSGS               5
#define MSG_QUEUE_MAX_MSG_SIZE               1024
#define MSG_QUEUE_NAME                       "/logger_task_mq"

#define MSG_MAX_LEN                          128
#define MSG_BUFF_MAX_LEN                     1024

#define SOCKET_HB_PORT_NUM                   8660
#define SOCKET_HB_LISTEN_QUEUE_SIZE          10

#define MAX_MSG_LEN							 512

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

/*----------------------------------- MACROS --------------------------------*/

/*---------------------------------- GLOBALS --------------------------------*/
mqd_t logger_mq_handle;
pthread_t uart_rx_thread_id, socket_hb_thread_id;

sig_atomic_t g_sig_kill_uart_rx_thread, g_sig_kill_sock_hb_thread;

int socket_task_initialized;

/* UART configuration variables */
struct termios *uart4_config;
char *uart4_port = "/dev/ttyO4";
int uart4_fd;

/*---------------------------------- GLOBALS --------------------------------*/

/*---------------------------- ENUMERATIONS ----------------------*/

/*---------------------------- STRUCTURES----------------------*/
/* Socket message structure */
typedef struct {
    uint32_t log_level;
    uint32_t log_type;
    uint32_t source_id;
	uint32_t data;
} sock_msg;

/*---------------------------- FUNCTION PROTOTYPES --------------------------*/
/**
 *  @brief  Create the main socket and hearbeat socket threads for the socket 
 *          task
 *
 *  The socket task is made multi-threaded with
 *     1. socket thread responsible for getting the sensor data received
 *        over the socket from Tiva and forward it to other tasks.
 *     2. socket heartbeat responsible for communicating with main task,
 *        to log heartbeat every time its requested by main task.
 *
 *  @param void
 *
 *  @return 0  : thread creation success
 *          -1 : thread creation failed
 *
 */
int create_threads(void);

/**
 *  @brief Entry point and executing entity for uart receive threas
 *
 *  The uart receive thread waits for any data received from the Tiva board
 *
 *  @param arg : argument to start_routine
 *
 *  @return void
 *
 */
void *uart_rx_thread_func(void *arg);

/**
 *  @brief Entry point and executing entity for socket heartbeat thread
 *
 *  The socket thread for heartbeat starts execution by invoking this function(start_routine)
 *
 *  @param arg : argument to start_routine
 *
 *  @return void
 *
 */
void *socket_hb_thread_func(void *arg);

/**
 *  @brief Entry point for configuring UART port
 *
 *  Configuration for UART port
 *
 *  @param uart_conf : uart configuration variable
 *         uart_desc:  uart file descriptor
 *
 *  @return void
 *
 */
void config_uart_port(struct termios *uart_conf, int uart_desc);

/**
 *  @brief Entry point for intializing UART
 *
 *  @param void
 *
 *  @return void
 *
 */
void uart4_init(void);

/**
 *  @brief Create the socket and initialize
 *
 *  This function create the socket for the given socket id.
 *
 *  @param sock_fd              : socket file descriptor
 *         server_addr_struct   : server address of the socket
 *         port_num             : port number in which the socket is communicating
 *         listen_qsize         : number of connections the socket is accepting
 *
 *  @return void
*/
void init_sock(int *sock_fd, struct sockaddr_in *server_addr_struct,
               int port_num, int listen_qsize);

/**
 *  @brief Signal handler for decision task
 *
 *  This function handles the reception of SIGKILL and SIGINT signal to the
 *  decision task and terminates all the threads and logger message queue handle and exits.
 *
 *  @param sig_num              : signal number
 *
 *  @return void
*/

void sig_handler(int sig_num);
#endif // _DECISION_TASK_H_
