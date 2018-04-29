/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/08/2018
* File:         comm_task.h
* Description:  Header file containing the macros, structs/enums, globals
                and function prototypes for source file comm_task.c
*************************************************************************/

#ifndef _COMM_TASK_H_
#define _COMM_TASK_H_

/*---------------------------------- INCLUDES -------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

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
#define USE_UART_FOR_COMM                   1

// Message queue attribute macros 
#define MSG_QUEUE_MAX_NUM_MSGS              5
#define MSG_QUEUE_MAX_MSG_SIZE              1024
#define LOGGER_MSG_QUEUE_NAME               "/logger_task_mq"
#define DECISION_MSG_QUEUE_NAME             "/decision_task_mq"

#define MSG_MAX_LEN                         128
#define MSG_BUFF_MAX_LEN                    1024

#define SOCKET_HB_PORT_NUM                  8660
#define SOCKET_HB_LISTEN_QUEUE_SIZE         10

#define SERVER_PORT_NUM 			        8500
#define SERVER_LISTEN_QUEUE_SIZE	        100

#define SOCK_REQ_MSG_API_MSG_LEN            64

#define BUFF_SIZE                           1024

#define LOGGER_ATTR_LEN                     32

/* Source ID definations */
#define TASK_PEDOMETER                      0x1
#define TASK_HUMIDITY                       0x2

#define TIVA_MSG_TIMESTAMP_ARR_LEN          9

/* Task Defines */
#define TASK_PEDOMETER                      0x1
#define TASK_HUMIDITY                       0x2
#define TASK_MAIN                           0x3
#define EXTERNAL_APP                        0x4

/* Log type defines */
#define LOG_TYPE_DATA                       0x1
#define LOG_TYPE_ERROR                      0x2
#define LOG_TYPE_REQUEST                    0x3
#define LOG_TYPE_RESPONSE                   0x4
#define LOG_HEARTBEAT                       0x5

/* Log level defines */
#define LOG_LEVEL_INFO                      0x1
#define LOG_LEVEL_STARTUP                   0x2
#define LOG_LEVEL_SHUTDOWN                  0x3
#define LOG_LEVEL_CRITICAL                  0x4

/*----------------------------------- MACROS --------------------------------*/

/*---------------------------------- GLOBALS --------------------------------*/
mqd_t logger_mq_handle, decision_mq_handle;
pthread_t comm_thread_id, socket_hb_thread_id, ext_app_int_thread_id;

sig_atomic_t g_sig_kill_comm_thread, g_sig_kill_sock_hb_thread, g_sig_kill_ext_app_sock_int_thread;

int server_sockfd, accept_conn_id;
struct sockaddr_in server_addr;
int comm_task_initialized;

#ifdef USE_UART_FOR_COMM
/* UART configuration variables */
struct termios *uart4_config;
char *uart4_port = "/dev/ttyO4";
int uart4_fd;

bool gb_waiting_for_ext_app_uart_rsp;

/* This semaphore will be used to signal the external app socket interface 
 * thread from communication thread that a response has been received for 
 * the request that was received from the external application */
sem_t sem_ext_app_req_rsp;
sem_t sem_sock_msg_shared;

#endif

/*---------------------------------- GLOBALS --------------------------------*/

/*---------------------------- STRUCTURES/ENUMERATIONS ----------------------*/
/* Socket message structure */
typedef struct 
{
    uint32_t log_level;
    uint32_t log_type;
    uint32_t source_id;
	uint32_t data;
    char timestamp[TIVA_MSG_TIMESTAMP_ARR_LEN];
} sock_msg;

struct _ext_app_req_msg_struct_
{
    int req_recipient;
    int params;
    char req_api_msg[SOCK_REQ_MSG_API_MSG_LEN];
};

struct _logger_msg_struct_
{
    char message[MSG_MAX_LEN];
    char logger_msg_src_id[LOGGER_ATTR_LEN];
    char logger_msg_level[LOGGER_ATTR_LEN];
};

sock_msg x_sock_data_rcvd_shared;

/*---------------------------- STRUCTURES/ENUMERATIONS ----------------------*/

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
 *  @brief Entry point and executing entity for main communication thread
 *
 *  The main comm thread starts execution by invoking this function(start_routine)
 *
 *  @param arg : argument to start_routine
 *
 *  @return void
 *
 */
void *comm_thread_func(void *arg);

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
 *  @brief Entry point and executing entity for external application socket interface thread
 *
 *  The external application socket interface thread starts execution by invoking this 
 *  function(start_routine)
 *
 *  @param arg : argument to start_routine
 *
 *  @return void
 *
 */
void *ext_app_int_thread_func(void *arg);

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

#ifdef USE_UART_FOR_COMM
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
#endif

/**
 *  @brief Initialize server socket
 *
 *  For an external application to communicate with the system, the socket
 *  task creates and listens on a socket for messages exposed to the external
 *  application. This function creates, binds and makes the socket task listen
 *  on this socket for messages from external application
 *
 *  @param sock_addr_struct        : sockaddr_in structre pointer
 *  @param port_num                : port number associated with the socket
 *  @param listen_queue_size       : backlog argument for listen system call
 *
 *  @return void
 *
 */
void initialize_server_socket(struct sockaddr_in *sock_addr_struct, 
                                int port_num, int listen_queue_size);

/**
 *  @brief Post data to logger task message queue
 *
 *  This function writes the messgae received from a Tiva task to the logger message
 *  queue
 *
 *  @param x_sock_data     : socket message structure
 *
 *  @return void
*/
void post_data_to_logger_task_queue(sock_msg x_sock_data);

/**
 *  @brief Post data to decision task message queue
 *
 *  This function writes the message received from a Tiva task to the decision task 
 *  message queue
 *
 *  @param x_sock_data     : socket message structure
 *
 *  @return void
*/
void post_data_to_decision_task_queue(sock_msg x_sock_data);

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

/*---------------------------- FUNCTION PROTOTYPES --------------------------*/

#endif // _COMM_TASK_H_
