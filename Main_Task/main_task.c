/***************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/07/2018
* File:         main_task.c
* Description:  Source file containing the functionality and implementation
*               of the main task
***************************************************************************/

#include "main_task.h"

int main(void)
{
	char buffer[BUFF_SIZE];

    create_sub_processes();

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("SigHandler setup for SIGINT failed\n");

    if (signal(SIGUSR1, sig_handler) == SIG_ERR)
        printf("SigHandler setup for SIGKILL failed\n");

    sleep(3);

    /* Create and initialize logger task socket */
    initialize_sub_task_socket(&logger_task_sockfd, &logger_task_sock_addr, LOGGER_TASK_PORT_NUM);

    if (connect(logger_task_sockfd, (struct sockaddr *)&logger_task_sock_addr, sizeof(logger_task_sock_addr)) < 0)
    {
        printf("\nConnection Failed for logger task \n");
        return -1;
    }

    /* Create and initialize socket task socket */
    initialize_sub_task_socket(&socket_task_sockfd, &socket_task_sock_addr, SOCKET_TASK_PORT_NUM);

    if (connect(socket_task_sockfd, (struct sockaddr *)&socket_task_sock_addr, sizeof(socket_task_sock_addr)) < 0)
    {
        printf("\nConnection Failed for socket task \n");
        return -1;
    }

    /* Create and initialize decision task socket */
    initialize_sub_task_socket(&decision_task_sockfd, &decision_task_sock_addr, DECISION_TASK_PORT_NUM);

    if (connect(decision_task_sockfd, (struct sockaddr *)&decision_task_sock_addr, sizeof(decision_task_sock_addr)) < 0)
    {
        printf("\nConnection Failed for decision task \n");
        return -1;
    }

    sleep(2);

    printf("Performing system start-up test\n");
    perform_startup_test();

    while(!g_kill_main_task)
    {
        check_status_of_sub_tasks();

        sleep(10);
    }

	return 0;	
}

void create_sub_processes(void)
{
    char sub_process_name[SUB_PROC_NAME_STR_LEN];

    FILE *fp_pid_file = fopen("pid_info_file.txt", "r");
    if (fp_pid_file)
    {
        fclose(fp_pid_file);
        remove("pid_info_file.txt");
    }
    
    /* Creating logger task */
    memset(sub_process_name, '\0', sizeof(sub_process_name));
    strcpy(sub_process_name, "logger"); 
    create_sub_process(sub_process_name);
  
    /* Creating socket task */
    memset(sub_process_name, '\0', sizeof(sub_process_name));
    strcpy(sub_process_name, "socket"); 
    create_sub_process(sub_process_name);
    
    /* Creating decision task */
    memset(sub_process_name, '\0', sizeof(sub_process_name));
    strcpy(sub_process_name, "decision"); 
    create_sub_process(sub_process_name);

}

void create_sub_process(char *process_name)
{
    pid_t child_pid;

    child_pid = fork();

    if (child_pid == 0)
    {
        /* Child Process */
        if (!strcmp(process_name, "socket"))
        {    
            write_pid_to_file(process_name, getpid());
            printf("Creating socket task\n");
            char *args[]={"./socket_task", "&", NULL};
            execvp(args[0],args);
        }
        else if (!strcmp(process_name, "logger"))
        {    
            write_pid_to_file(process_name, getpid());
            printf("Creating logger task\n");
            char *args[]={"./logger_task", "&", NULL};
            execvp(args[0],args);
        }
        else if (!strcmp(process_name, "decision"))
        {    
            write_pid_to_file(process_name, getpid());
            printf("Creating decision task\n");
            char *args[]={"./decision_task", "&", NULL};
            execvp(args[0],args);
        }
    }
    else if (child_pid > 0)
    {
        /* Parent Process */
        /* We are just returning back to main function in the parent process */
    }
    else 
    {
        printf("fork failed while creating child process for %s task\n", process_name);
        perror("fork failed");
    }
  
    return;

}

void write_pid_to_file(char *proc_name, pid_t child_pid)
{
    FILE *fp_pid_file = fopen("pid_info_file.txt", "r");
    char pid_info_str[64];
    memset(pid_info_str, '\0', sizeof(pid_info_str));
    
    if (fp_pid_file == NULL)
    {
        fp_pid_file = fopen("pid_info_file.txt", "w");
        
        sprintf(pid_info_str, "%s task: %d\n", proc_name, (int)child_pid);
        fwrite(pid_info_str, strlen(pid_info_str), sizeof(char), fp_pid_file);
        
        fclose(fp_pid_file);
    }
    else
    {
        fclose(fp_pid_file);
        fp_pid_file = fopen("pid_info_file.txt", "a");
        
        sprintf(pid_info_str, "%s task: %d\n", proc_name, (int)child_pid);
        fwrite(pid_info_str, strlen(pid_info_str), sizeof(char), fp_pid_file);
        
        fclose(fp_pid_file);
    }
}

void initialize_sub_task_socket(int *sock_fd, struct sockaddr_in *sock_addr_struct, int port_num)
{
	memset(sock_addr_struct, '0', sizeof(struct sockaddr_in));
	sock_addr_struct->sin_family = AF_INET;
	sock_addr_struct->sin_port = htons(port_num);

    if ((*sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    int option = 1;
    if(setsockopt(*sock_fd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR),
                (char*)&option,sizeof(option)) < 0)
    {
        perror("setsockopt for socket reusability failed");
        close(*sock_fd);
        exit(EXIT_FAILURE);
    }

    struct timeval rcv_timeout;
    rcv_timeout.tv_sec = SOCK_SEND_RCV_TIMEOUT;
    rcv_timeout.tv_usec = 0;
    if (setsockopt(*sock_fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&rcv_timeout,sizeof(struct timeval)) < 0)
    {
        perror("setsockopt for send timeout set failed");
        close(*sock_fd);
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(*sock_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&rcv_timeout,sizeof(struct timeval)) < 0)
    {
        perror("setsockopt for recv timeout set failed");
        close(*sock_fd);
        exit(EXIT_FAILURE);
    }

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, SENSOR_TASK_SOCK_IP_ADDR, &(sock_addr_struct->sin_addr))<=0)
    {
        perror("inet_pton failed");
        printf("\nInvalid address/ Address not supported for temperature task\n");
        exit(EXIT_FAILURE);
    }
}

void check_status_of_sub_tasks(void)
{
    /* Check if socket task is alive */
    check_subtask_status(socket_task_sockfd, "Socket"); 

    /* Check if logger task is alive */
    check_subtask_status(logger_task_sockfd, "Logger");
    
    /* Check if decision task is alive */
    check_subtask_status(decision_task_sockfd, "Decision"); 
}

void check_subtask_status(int sock_fd, char *task_name)
{
    char recv_buffer[BUFF_SIZE];
    char send_buffer[] = "heartbeat";
    
    memset(recv_buffer, '\0', sizeof(recv_buffer));

    ssize_t num_sent_bytes = send(sock_fd, send_buffer, sizeof(send_buffer), 0);
    if (num_sent_bytes < 0)
        perror("send failed");

    ssize_t num_recv_bytes = recv(sock_fd, recv_buffer, sizeof(recv_buffer), 0);
    if (num_recv_bytes < 0)
        perror("recv failed");

    if (!strcmp(recv_buffer, "Alive"))
        printf("%s task alive\n", task_name);
    else
    {
        if (!strcmp(task_name, "Logger"))
        {
            logger_task_unalive_count++;

            if (logger_task_unalive_count >= LOGGER_TASK_UNALIVE_CNT_LOG_LIMIT)
                log_task_unalive_msg_to_log_file(task_name);
        }
        else if (!strcmp(task_name, "Socket"))
        {
            socket_task_unalive_count++;

            if (socket_task_unalive_count >= SOCK_TASK_UNALIVE_CNT_LOG_LIMIT)
                log_task_unalive_msg_to_log_file(task_name);
        }
        else if (!strcmp(task_name, "Decision"))
        {
            decision_task_unalive_count++;

            if (decision_task_unalive_count >= DECISION_TASK_UNALIVE_CNT_LOG_LIMIT)
                log_task_unalive_msg_to_log_file(task_name);
        }
    }
}

void perform_startup_test(void)
{
    /* The startup test will check if the sub-tasks on BBG have been started
     * and initialized successfully or not 
    */

    /* Check the logger task */
    int logger_task_st_status = perform_sub_task_startup_test(logger_task_sockfd, "logger");
    if (logger_task_st_status != 0)
        stop_entire_system();
    
    /* Check the socket task */
    int socket_task_st_status = perform_sub_task_startup_test(socket_task_sockfd, "socket");
    if (socket_task_st_status != 0)
        stop_entire_system();
    
    /* Check the decision task */
    int decision_task_st_status = perform_sub_task_startup_test(decision_task_sockfd, "decision");
    if (decision_task_st_status != 0)
        stop_entire_system();
}

int perform_sub_task_startup_test(int sock_fd, char *proc_name)
{
    char recv_buffer[BUFF_SIZE];
    char send_buffer[] = "startup_check";
    
    memset(recv_buffer, '\0', sizeof(recv_buffer));

    ssize_t num_sent_bytes = send(sock_fd, send_buffer, sizeof(send_buffer), 0);
    if (num_sent_bytes < 0)
        perror("send failed");

    ssize_t num_recv_bytes = recv(sock_fd, recv_buffer, sizeof(recv_buffer), 0);
    if (num_recv_bytes < 0)
        perror("recv failed");

    if (!strcmp(recv_buffer, "Initialized"))
    {
        printf("%s sensor is initialized\n", proc_name);
        return 0;
    }
    else if (!strcmp(recv_buffer, "Uninitialized"))
    {
        printf("%s sensor isn't initalized\n", proc_name);
        return -1;
    }
    else
    {
        printf("Message received on socket : %s unknown\n", recv_buffer);
        return -1;
    }
}

void stop_entire_system(void)
{
    kill_already_created_processes();

    /* Turn on USR led to indicate that a failure has occurred */
    turn_on_usr_led();

    exit(1);
}

void kill_already_created_processes(void)
{
    printf("Killing already created processes\n");
    
    FILE *fp_pid_info_file = fopen("./pid_info_file.txt", "r");
    if (fp_pid_info_file == NULL)
    {
        perror("file open failed");
        printf("File %s open failed\n", "pid_info_file.txt");
        return;
    }

    char *buffer;
    size_t num_bytes = 120;
    char colon_delimiter[] = ":";
    ssize_t bytes_read;

    buffer = (char *)malloc(num_bytes*sizeof(char));

    while ((bytes_read = getline(&buffer, &num_bytes, fp_pid_info_file)) != -1)
    {
        char *token = strtok(buffer, colon_delimiter);

        if (!strcmp(token, "temperature task"))
        {
            token = strtok(NULL, colon_delimiter);
            printf("Killing temperature task\n");
            int pid_to_kill = atoi(token);
            
            /* We wanted to kill the temperature process here by sending a SIGKILL,
            ** but since we could not setup a signal handler for SIGKILL, we are 
            ** sending a SIGSTOP instead and trying to handle SIGUSR1 in the 
            ** temperature process */
            kill(pid_to_kill, SIGUSR1);
            
            int status;
            pid_t end_id = waitpid(pid_to_kill, &status, 0);
            if (end_id == pid_to_kill)
            {
                if (WIFEXITED(status))
                    printf("Temperature task successfully killed\n");
            }
            else
            {
                perror("Temperature task: waitpid error\n");
            }
        }
        else if (!strcmp(token, "light task"))
        {
            token = strtok(NULL, colon_delimiter);
            printf("Killing light task\n");
            int pid_to_kill = atoi(token);
            
            kill(pid_to_kill, SIGUSR1);
            
            int status;
            pid_t end_id = waitpid(pid_to_kill, &status, 0);
            if (end_id == pid_to_kill)
            {
                if (WIFEXITED(status))
                    printf("Light task successfully killed\n");
            }
            else
            {
                perror("Light task: waitpid error\n");
            }

        }
        else if (!strcmp(token, "logger task"))
        {
            token = strtok(NULL, colon_delimiter);
            printf("Killing logger task\n");
            int pid_to_kill = atoi(token);
            
            kill(pid_to_kill, SIGUSR1);
            
            int status;
            pid_t end_id = waitpid(pid_to_kill, &status, 0);
            if (end_id == pid_to_kill)
            {
                if (WIFEXITED(status))
                    printf("Logger task successfully killed\n");
            }
            else
            {
                perror("Logger task: waitpid error\n");
            }
        }
        else if (!strcmp(token, "socket task"))
        {
            token = strtok(NULL, colon_delimiter);
            printf("Killing socket task\n");
            int pid_to_kill = atoi(token);
            
            kill(pid_to_kill, SIGUSR1);
            
            int status;
            pid_t end_id = waitpid(pid_to_kill, &status, 0);
            if (end_id == pid_to_kill)
            {
                if (WIFEXITED(status))
                    printf("Socket task successfully killed\n");
            }
            else
            {
                perror("Socket task: waitpid error\n");
            }
        }
    }

    if (buffer)
        free(buffer);

    if (fp_pid_info_file)
        fclose(fp_pid_info_file);
}

void turn_on_usr_led(void)
{
    printf("Turning on USR led\n");
   
#if 0
    FILE *fp_brightness_file = fopen("/sys/class/leds/beaglebone:green:usr2/brightness", "w");
    if (fp_brightness_file == NULL)
    {
        perror("fopen failed");
        printf("Failed to open brightness file\n");
        return;
    }

    int on_value = 1;

    fwrite(&on_value, 1, sizeof(int), fp_brightness_file);

    fclose(fp_brightness_file);
#endif

    char led_turn_on_cmd[128];
    memset(led_turn_on_cmd, '\0', sizeof(led_turn_on_cmd));

    sprintf(led_turn_on_cmd, "sudo sh -c 'echo 1 > /sys/class/leds/beaglebone:green:usr3/brightness'");
    system(led_turn_on_cmd);
}

void log_task_unalive_msg_to_log_file(char *task_name)
{
    int msg_priority;

    /* Set the message queue attributes */
    struct mq_attr logger_mq_attr = { .mq_flags = 0,
                                      .mq_maxmsg = MSG_QUEUE_MAX_NUM_MSGS,  // Max number of messages on queue
                                      .mq_msgsize = MSG_QUEUE_MAX_MSG_SIZE  // Max. message size
                                    };

    logger_mq_handle = mq_open(MSG_QUEUE_NAME, O_RDWR, S_IRWXU, &logger_mq_attr);

    char main_task_data_msg[MSG_MAX_LEN];
    memset(main_task_data_msg, '\0', sizeof(main_task_data_msg));

    sprintf(main_task_data_msg, "%s task not alive", task_name);
  
    struct _logger_msg_struct_ logger_msg;
    memset(&logger_msg, '\0', sizeof(logger_msg));
    strcpy(logger_msg.message, main_task_data_msg);
    strncpy(logger_msg.logger_msg_src_id, "Main", strlen("Main"));                                    
    logger_msg.logger_msg_src_id[strlen("Main")] = '\0';                                              
    strncpy(logger_msg.logger_msg_level, "Error", strlen("Error"));                                     
    logger_msg.logger_msg_level[strlen("Error")] = '\0';

    msg_priority = 1;
    int num_sent_bytes = mq_send(logger_mq_handle, (char *)&logger_msg, 
                            sizeof(logger_msg), msg_priority);
    if (num_sent_bytes < 0)
        perror("mq_send failed");
}

void sig_handler(int sig_num)
{
    char buffer[MSG_BUFF_MAX_LEN];
    memset(buffer, '\0', sizeof(buffer));

    if (sig_num == SIGINT || sig_num == SIGUSR1)
    {
        if (sig_num == SIGINT)
            printf("Caught signal %s in temperature task\n", "SIGINT");
        else if (sig_num == SIGUSR1)
            printf("Caught signal %s in temperature task\n", "SIGKILL");

        kill_already_created_processes();
        
        mq_close(logger_mq_handle);
        
        g_kill_main_task = 1;
    }
}
