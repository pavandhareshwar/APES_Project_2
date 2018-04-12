
/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/07/2018
* File:         main_task.h
* Description:  Header file containing the macros, structs/enums, globals
                and function prototypes for source file main_task.c
*************************************************************************/

#ifndef _MAIN_TASK_H_
#define _MAIN_TASK_H_

/*---------------------------------- INCLUDES -------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <mqueue.h>

#include <semaphore.h>

/*---------------------------------- INCLUDES -------------------------------*/

/*----------------------------------- MACROS --------------------------------*/
#define SUB_PROC_NAME_STR_LEN               32

#define LOGGER_TASK_PORT_NUM 			    8650
#define LOGGER_TASK_QUEUE_SIZE			    10

#define SOCKET_TASK_PORT_NUM 			    8660
#define SOCKET_TASK_QUEUE_SIZE			    10

#define DECISION_TASK_PORT_NUM 			    8670
#define DECISION_TASK_QUEUE_SIZE			10

#define SENSOR_TASK_SOCK_IP_ADDR            "127.0.0.1"   

#define BUFF_SIZE                           1024
#define MSG_MAX_LEN                         128
#define MSG_BUFF_MAX_LEN                    1024

#define MSG_QUEUE_NAME                      "/logger_task_mq"
#define MSG_QUEUE_MAX_NUM_MSGS              5
#define MSG_QUEUE_MAX_MSG_SIZE              1024

#define LOGGER_TASK_UNALIVE_CNT_LOG_LIMIT   5
#define SOCK_TASK_UNALIVE_CNT_LOG_LIMIT     5
#define DECISION_TASK_UNALIVE_CNT_LOG_LIMIT 5

#define LOGGER_ATTR_LEN                     32

#define SOCK_SEND_RCV_TIMEOUT               5
/*----------------------------------- MACROS --------------------------------*/

/*---------------------------------- GLOBALS --------------------------------*/
int socket_task_sockfd, logger_task_sockfd, decision_task_sockfd;

struct sockaddr_in socket_task_sock_addr, logger_task_sock_addr, decision_task_sock_addr;

int logger_task_unalive_count, socket_task_unalive_count, decision_task_unalive_count;

mqd_t logger_mq_handle;

sig_atomic_t g_kill_main_task;

/*---------------------------------- GLOBALS --------------------------------*/

/*---------------------------- STRUCTURES/ENUMERATIONS ----------------------*/
struct _logger_msg_struct_                                                                            
{                                                                                                     
    char message[MSG_MAX_LEN]; 
    char logger_msg_src_id[LOGGER_ATTR_LEN];
    char logger_msg_level[LOGGER_ATTR_LEN];
}; 

/*---------------------------- STRUCTURES/ENUMERATIONS ----------------------*/

/*---------------------------- FUNCTION PROTOTYPES --------------------------*/

/**
 *  @brief Initialize sub tasks interface socket
 *  
 *  For the main task to check the status of each of the remaining tasks, it 
 *  sends a heartbeat message to each of these tasks and when it receives a
 *  reply, it knows that the task is alive. For the main task to check the 
 *  status, it uses socket as an IPC mechanism.
 *
 *  This function creates a socket between main task and the sensor task for
 *  communication.
 *
 *  @param sock_fd                 : pointer socket file descriptor
 *  @param sock_addr_struct        : sockaddr_in structure pointer
 *  @param port_num                : port number associated with the socket
 *
 *  @return void
 *
 */
void initialize_sub_task_socket(int *sock_fd, struct sockaddr_in *sock_addr_struct, 
                                int port_num);

/**
 *  @brief Check status of a specified sub task 
 *  
 *  This function checks the status of the specified subtask to see if it 
 *  is alive
 *
 *  @param sock_fd                 : socket file descriptor for the task 
 *  @param task_name               : name of the subtask
 *
 *  @return void
 *
 */
void check_subtask_status(int sock_fd, char *task_name);

/**
 *  @brief Check status of sub tasks 
 *  
 *  This function checks the status of each of the subtasks to see if they 
 *  are alive
 *
 *  @param void
 *
 *  @return void
 *
 */
void check_status_of_sub_tasks(void);

/**
 *  @brief Log unalive message to logger task message queue 
 *  
 *  This function logs a message to the logger task message queue if a certain
 *  process isn't alive when checked for a predefined number of times
 *
 *  @param task_name               : name of the subtask
 *
 *  @return void
 *
 */
void log_task_unalive_msg_to_log_file(char *task_name);

/**
 *  @brief Create sub processes 
 *  
 *  This function creates the temperature, light, logger and socket sub-procesess
 *
 *  @param void
 *
 *  @return void
 *
 */
void create_sub_processes(void);

/**
 *  @brief Create a specific sub process 
 *  
 *  This function creates a new task as per the name specified by @param task_name
 *
 *  @param task_name                 : name of the subtask 
 *
 *  @return void
 *
 */
void create_sub_process(char *process_name);

/**
 *  @brief Perform start-up tests
 *  
 *  This function performs the start-up tests to ensure that the hardware, processes 
 *  and communication primitivies are working. If any of the start-up test fails, the
 *  already existing processes and threads are killed and some cleanup is done
 *
 *  @param void
 *
 *  @return void
 *
 */
void perform_startup_test(void);

/**
 *  @brief Perform start-up test for a sub task
 *  
 *  This function performs the start-up tests to ensure that the hardware, threads 
 *  and communication primitives of the specified sub task are working.
 *
 *  @param sock_fd                     : socket file descriptor
 *  @param proc_name                   : process name
 *
 *  @return void
 *
 */
int perform_sub_task_startup_test(int sock_fd, char *proc_name);

/**
 *  @brief Stop entire system 
 *  
 *  This function is called when a certain start-up test fails and performs some 
 *  clean-up and exits. 
 *
 *  @param void
 *
 *  @return void
 *
 */
void stop_entire_system(void);

/**
 *  @brief Kill already created processes 
 *  
 *  This function kills all the created processes by the main task before the start-up
 *  test is triggered, as part of clean-up and exit of the entire system.
 *
 *  @param void
 *
 *  @return void
 *
 */
void kill_already_created_processes(void);

/**
 *  @brief Turn on user led 
 *  
 *  This function turns on a user led on the beagle bone green to indicate of the 
 *  system failure to start-up
 *
 *  @param void
 *
 *  @return void
 *
 */
void turn_on_usr_led(void);

/**
 *  @brief Write pid of created processes to a file 
 *  
 *  This function writes the pid of the sub processes created by main task to a file
 *
 *  @param proc_name                   : name of the child process
 *  @param child_pid                   : pid of the child process
 *
 *  @return void
 *
 */
void write_pid_to_file(char *proc_name, pid_t child_pid);

/**
 *  @brief Signal handler for main task
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
/*---------------------------- FUNCTION PROTOTYPES --------------------------*/

#endif // _MAIN_TASK_H_
