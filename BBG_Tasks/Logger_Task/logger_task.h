/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/18/2018
* File:         logger_task.h
* Description:  Header file containing the macros, structs/enums, globals
                and function prototypes for source file logger_task.c
*************************************************************************/

#ifndef _LOGGER_TASK_H_
#define _LOGGER_TASK_H_

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
#define MSG_QUEUE_MAX_NUM_MSGS              5
#define MSG_QUEUE_MAX_MSG_SIZE              1024
#define MSG_QUEUE_NAME                      "/logger_task_mq"

#define LOGGER_TASK_CONF_FILE_PATH          "./Logger_Task/logger_task_conf_file.txt"
#define LOGGER_FILE_PATH                    "./"
#define LOGGER_FILE_NAME                    "logger_file.txt"
#define LOGGER_FILE_NAME_MAX_LEN            128

#define LOG_MSG_PAYLOAD_SIZE                256
#define MSG_MAX_LEN                         128

#define MSG_BUFF_MAX_LEN                    1024

#define LOGGER_FILE_PATH_LEN                256           
#define LOGGER_FILE_NAME_LEN                64           

#define SOCKET_HB_PORT_NUM                  8650
#define SOCKET_HB_LISTEN_QUEUE_SIZE         10

#define LOGGER_ATTR_LEN                     32

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
int logger_fd;
pthread_t logger_thread_id, socket_hb_thread_id;

sig_atomic_t g_sig_kill_logger_thread, g_sig_kill_sock_hb_thread;

int logger_task_initialized = 0;

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
 *  @brief Initialize the logger task
 *  
 *  This funcition will create the message queue for logger task and
 *  open a file handle of logger file for writing. (If the logger file
 *  already exists, it is deleted and a fresh one is created). 
 *
 *  @param void
 *
 *  @return 0  : if sensor initialization is a success
            -1 : if sensor initialization fails
*/
int logger_task_init();

/**
 *  @brief Read from configuration file for the logger task
 *
 *  This function reads the configuration parameters for the logger task file
 *  and sets-up the logger file as per this configuration
 *
 *  @param file     : name of the config file
 *
 *  @return void
 *
 */
int read_logger_conf_file(char *file);

/**
 *  @brief Create logger and hearbeat socket threads for logger task
 *
 *  The logger task is made multi-threaded with
 *     1. logger thread responsible for reading messages from its message queue
 *        and logging it to a file.
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
 *  @brief Entry point and executing entity for logger thread
 *
 *  The logger thread starts execution by invoking this function(start_routine)
 *
 *  @param arg : argument to start_routine
 *
 *  @return void
 *
 */
void *logger_thread_func(void *arg);

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

#if 0
void write_test_msg_to_logger();
#endif

/**
 *  @brief Read message from logger message queue
 *
 *  This function will read messages from its message queue and log it to a file 
 *
 *  @param void
 *
 *  @return void
*/
void read_from_logger_msg_queue(void);

/**
 *  @brief Flush logger message queue
 *
 *  This function will check the logger task message queue and flush the messages to 
 *  the log file
 *
 *  @param void
 *
 *  @return void
*/
void flush_logger_mq(void);

/**
 *  @brief Cleanup of the logger sensor
 *
 *  This function will close the message queue and the logger file handle 
 *
 *  @param void
 *
 *  @return void
*/
void logger_task_exit(void);

/**
 *  @brief Signal handler for temperature task
 *
 *  This function handles the reception of SIGKILL and SIGINT signal to the
 *  temperature task and terminates all the threads, closes the I2C file descriptor
 *  and logger message queue handle and exits.
 *
 *  @param sig_num              : signal number
 *
 *  @return void
*/

void sig_handler(int sig_num);
#endif // _LOGGER_TASK_H_
