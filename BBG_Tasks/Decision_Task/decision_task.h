/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/08/2018
* File:         decision_task.h
* Description:  Header file containing the macros, structs/enums, globals
                and function prototypes for source file decision_task.c
*************************************************************************/

#ifndef _DECISION_TASK_H_
#define _DECISION_TASK_H_

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
/*---------------------------------- INCLUDES -------------------------------*/

/*----------------------------------- MACROS --------------------------------*/
// Message queue attribute macros 
#define MSG_QUEUE_MAX_NUM_MSGS               5
#define MSG_QUEUE_MAX_MSG_SIZE               1024
#define DECISION_MSG_QUEUE_NAME              "/decision_task_mq"
#define LOGGER_MSG_QUEUE_NAME                "/logger_task_mq"

#define MSG_MAX_LEN                          128
#define MSG_BUFF_MAX_LEN                     1024

#define SOCKET_HB_PORT_NUM                   8670
#define SOCKET_HB_LISTEN_QUEUE_SIZE          10

#define TASK_PEDOMETER                      0x1
#define TASK_HUMIDITY                       0x2

/*----------------------------------- MACROS --------------------------------*/

/*---------------------------------- GLOBALS --------------------------------*/
mqd_t decision_mq_handle, logger_mq_handle;
pthread_t decision_thread_id, socket_hb_thread_id;

sig_atomic_t g_sig_kill_decision_thread, g_sig_kill_sock_hb_thread;

int decision_task_initialized;

/*---------------------------------- GLOBALS --------------------------------*/

/*---------------------------- STRUCTURES/ENUMERATIONS ----------------------*/
struct _socket_msg_struct_                                                                            
{
    uint32_t log_level;
    uint32_t log_type;
    uint32_t source_id;
	uint32_t data;
};

/*---------------------------- STRUCTURES/ENUMERATIONS ----------------------*/

/*---------------------------- FUNCTION PROTOTYPES --------------------------*/
/**
 *  @brief Create decision and hearbeat socket threads for decision task
 *
 *  The decision task is made multi-threaded with
 *     1. decision thread responsible for decoding the sensor data received
 *        over the socket from Tiva and performing some decisiion actions.
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
 *  @brief Entry point and executing entity for decision thread
 *
 *  The decision thread starts execution by invoking this function(start_routine)
 *
 *  @param arg : argument to start_routine
 *
 *  @return void
 *
 */
void *decision_thread_func(void *arg);

/**
 *  @brief Entry point and executing entity for socket thread
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
 *  @brief Read message from decision task message queue
 *
 *  This function will read messages from decision task message queue and process it to take 
 *  some decision
 *
 *  @param void
 *
 *  @return void
*/
void read_from_decision_task_msg_queue(void);

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
