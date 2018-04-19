/*************************************************************************
* Author:       Pavan Dhareshwar & Sridhar Pavithrapu
* Date:         04/08/2018
* File:         logger_task.c
* Description:  Source file describing the functionality and implementation
*               of logger task.
*************************************************************************/

#include "logger_task.h"

int main(void)
{

    logger_task_initialized = 0;

    int init_status = logger_task_init();
    if (init_status == -1)
    {
        printf("logger task initialization failed\n");
        exit(1);
    }

    //write_test_msg_to_logger();

    int thread_create_status = create_threads();
    if (thread_create_status)
    {
        printf("Thread creation failed\n");
    }
    else
    {
        printf("Thread creation success\n");
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("SigHandler setup for SIGINT failed\n");

    if (signal(SIGUSR1, sig_handler) == SIG_ERR)
        printf("SigHandler setup for SIGKILL failed\n");

    g_sig_kill_logger_thread = 0;
    g_sig_kill_sock_hb_thread = 0;

    pthread_join(logger_thread_id, NULL);
    pthread_join(socket_hb_thread_id, NULL);

    logger_task_exit();

    return 0;
}

int logger_task_init()
{
    /* In the logger task init function, we create the message queue */

    /* Set the message queue attributes */
    struct mq_attr logger_mq_attr = { .mq_flags = 0,
                                      .mq_maxmsg = MSG_QUEUE_MAX_NUM_MSGS,  // Max number of messages on queue
                                      .mq_msgsize = MSG_QUEUE_MAX_MSG_SIZE  // Max. message size 
                                    };


    logger_mq_handle = mq_open(MSG_QUEUE_NAME, O_CREAT | O_RDWR, S_IRWXU, &logger_mq_attr);
    if (logger_mq_handle < 0)
    {
        perror("Logger message queue create failed");
        return -1;
    }

    printf("Logger message queue successfully created\n");
    
    char filename[LOGGER_FILE_NAME_MAX_LEN];
    memset(filename, '\0', sizeof(filename));
    int conf_file_read_status = read_logger_conf_file(filename);
    if (conf_file_read_status != 0)
    {
        printf("Logger task config file read failed. Using default log file path and name\n");
        sprintf(filename, "%s%s", LOGGER_FILE_PATH, LOGGER_FILE_NAME);
    }

    if (open(filename, O_RDONLY) != -1)
    {
        printf("Logger file exists. Deleting existing file.\n");
        remove(filename);
        sync();
    }

    printf("Creating file %s\n", filename);
    logger_fd = creat(filename ,(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (logger_fd == -1)
    {
        perror("Logger file open failed");
        return -1;
    }
    else
    {
        printf("Logger file open success\n");
    }

    logger_task_initialized = 1;

    return 0;
}

int read_logger_conf_file(char *file)
{
    FILE *fp_conf_file = fopen(LOGGER_TASK_CONF_FILE_PATH, "r");
    if (fp_conf_file == NULL)
    {
        perror("file open failed");
        printf("File %s open failed\n", LOGGER_TASK_CONF_FILE_PATH);
        return -1;
    }

    char logger_file_path[LOGGER_FILE_PATH_LEN];
    char logger_file_name[LOGGER_FILE_NAME_LEN];
    char *buffer;
    size_t num_bytes = 120;
    char equal_delimiter[] = "=";
    ssize_t bytes_read;

    memset(logger_file_path, '\0', sizeof(logger_file_path));
    memset(logger_file_name, '\0', sizeof(logger_file_name));

    buffer = (char *)malloc(num_bytes*sizeof(char));

    while ((bytes_read = getline(&buffer, &num_bytes, fp_conf_file)) != -1)
    {
        char *token = strtok(buffer, equal_delimiter);

        if (!strcmp(token, "LOGGER_FILE_PATH"))
        {
            token = strtok(NULL, equal_delimiter);
            strcpy(logger_file_path, token);
            int len = strlen(logger_file_path);
            if (logger_file_path[len-1] == '\n')
                logger_file_path[len-1] = '\0';
        }
        else if (!strcmp(token, "LOGGER_FILE_NAME"))
        {
            token = strtok(NULL, equal_delimiter);
            strcpy(logger_file_name, token);
            int len = strlen(logger_file_name);
            if (logger_file_name[len-1] == '\n')
                logger_file_name[len-1] = '\0';
        }
    }

    strcpy(file, logger_file_path);
    strcat(file, logger_file_name);

    if (buffer)
        free(buffer);

    if (fp_conf_file)
        fclose(fp_conf_file);

    return 0;
}

int create_threads(void)
{
    int logger_t_creat_ret_val = pthread_create(&logger_thread_id, NULL, &logger_thread_func, NULL);
    if (logger_t_creat_ret_val)
    {
        perror("Sensor thread creation failed");
        return -1;
    }

    int sock_hb_t_creat_ret_val = pthread_create(&socket_hb_thread_id, NULL, &socket_hb_thread_func, NULL);
    if (sock_hb_t_creat_ret_val)
    {
        perror("Socket heartbeat thread creation failed");
        return -1;
    }

    return 0;
}

void *logger_thread_func(void *arg)
{
    while(!g_sig_kill_logger_thread)
    {
        /* This function will continously read from the logger task message 
        ** queue and write it to logger file */
        read_from_logger_msg_queue();
    }

    pthread_exit(NULL);
}

void *socket_hb_thread_func(void *arg)
{
    int sock_hb_fd;
    struct sockaddr_in sock_hb_address;
    int sock_hb_addr_len = sizeof(sock_hb_address);

    init_sock(&sock_hb_fd, &sock_hb_address, SOCKET_HB_PORT_NUM, SOCKET_HB_LISTEN_QUEUE_SIZE);


    int accept_conn_id;
    printf("Waiting for request...\n");
    if ((accept_conn_id = accept(sock_hb_fd, (struct sockaddr *)&sock_hb_address,
                    (socklen_t*)&sock_hb_addr_len)) < 0)
    {
        perror("accept failed");
        //pthread_exit(NULL);
    }

    char recv_buffer[MSG_BUFF_MAX_LEN];
    char send_buffer[] = "Alive";

    while (!g_sig_kill_sock_hb_thread)
    {
        memset(recv_buffer, '\0', sizeof(recv_buffer));

        size_t num_read_bytes = read(accept_conn_id, &recv_buffer, sizeof(recv_buffer));

        if (!strcmp(recv_buffer, "heartbeat"))
        {
			ssize_t num_sent_bytes = send(accept_conn_id, send_buffer, strlen(send_buffer), 0);
            if (num_sent_bytes < 0)
                perror("send failed");
        }
        else if (!strcmp(recv_buffer, "startup_check"))
        {
            /* For the sake of start-up check, because we have the temperature sensor initialized
            ** by the time this thread is spawned. So we perform a "get_temp_data" call to see if
            ** everything is working fine */
            if (logger_task_initialized == 1)
                strcpy(send_buffer, "Initialized");
            else
                strcpy(send_buffer, "Uninitialized");

            ssize_t num_sent_bytes = send(accept_conn_id, send_buffer, strlen(send_buffer), 0);
            if (num_sent_bytes < 0)
                perror("send failed");
        }
    }
}

#if 0
void write_test_msg_to_logger()
{
    struct _logger_msg_struct_ logger_msg = {0};

    const char test_msg[] = "Testing if msg queue comm works";
    strcpy(logger_msg.message, test_msg);
    logger_msg.msg_len = strlen(test_msg);

    logger_msg.logger_msg_type = MSG_TYPE_TEMP_DATA;

    int msg_priority = 1;
    int num_sent_bytes = mq_send(logger_mq_handle, (char *)&logger_msg, 
                                    sizeof(logger_msg), msg_priority);

    if (num_sent_bytes < 0)
        perror("mq_send failed");
}
#endif

void init_sock(int *sock_fd, struct sockaddr_in *server_addr_struct,
               int port_num, int listen_qsize)
{
    int serv_addr_len = sizeof(struct sockaddr_in);

    /* Create the socket */
    if ((*sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket creation failed");
        pthread_exit(NULL); // Change these return values from pthread_exit
    }

    int option = 1;
    if(setsockopt(*sock_fd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (void *)&option, sizeof(option)) < 0)
    {
        perror("setsockopt failed");
        pthread_exit(NULL);
    }

    server_addr_struct->sin_family = AF_INET;
    server_addr_struct->sin_addr.s_addr = INADDR_ANY;
    server_addr_struct->sin_port = htons(port_num);

    if (bind(*sock_fd, (struct sockaddr *)server_addr_struct,
								sizeof(struct sockaddr_in))<0)
    {
        perror("bind failed");
        pthread_exit(NULL);
    }

    if (listen(*sock_fd, listen_qsize) < 0)
    {
        perror("listen failed");
        pthread_exit(NULL);
    }

}

void read_from_logger_msg_queue(void)
{
    char recv_buffer[MSG_MAX_LEN];
    memset(recv_buffer, '\0', sizeof(recv_buffer));

    int msg_priority;
    
    int num_recv_bytes;
    while ((num_recv_bytes = mq_receive(logger_mq_handle, (char *)&recv_buffer,
                                    MSG_QUEUE_MAX_MSG_SIZE, &msg_priority)) != -1)
    {
        if (num_recv_bytes < 0)
        {
            perror("mq_receive failed");
            return;
        }

        time_t tval = time(NULL);
        struct tm *cur_time = localtime(&tval);
        
        char timestamp_str[32];
        memset(timestamp_str, '\0', sizeof(timestamp_str));

        sprintf(timestamp_str, "%02d:%02d:%02d", cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec);
        
        char msg_to_write[LOG_MSG_PAYLOAD_SIZE];
        memset(msg_to_write, '\0', sizeof(msg_to_write));
        
        char step_count_msg[32];
        memset(step_count_msg, '\0', sizeof(step_count_msg));
            
        sprintf(step_count_msg, "Message: Step count: %d\n", (((struct _socket_msg_struct_ *)&recv_buffer)->data));
        
        if ((((struct _socket_msg_struct_ *)&recv_buffer)->source_id) == TASK_PEDOMETER)
        {
            sprintf(msg_to_write, "Timestamp: %s | Message_Src: %s ",
                    timestamp_str, "Pedometer Task");

        }
        else if ((((struct _socket_msg_struct_ *)&recv_buffer)->source_id) == TASK_HEART_RATE)
        {
            sprintf(msg_to_write, "Timestamp: %s | Message_Src: %s ",
                    timestamp_str, "Heart Rate Task");

        }

        if ((((struct _socket_msg_struct_ *)&recv_buffer)->log_level) == LOG_LEVEL_INFO)
        {
            strcat(msg_to_write, "| Log_Level: LOG_LEVEL_INFO ");
        }
        else if ((((struct _socket_msg_struct_ *)&recv_buffer)->log_level) == LOG_LEVEL_STARTUP)
        {
            strcat(msg_to_write, "| Log_Level: LOG_LEVEL_STARTUP ");
        }
        else if ((((struct _socket_msg_struct_ *)&recv_buffer)->log_level) == LOG_LEVEL_SHUTDOWN)
        {
            strcat(msg_to_write, "| Log_Level: LOG_LEVEL_SHUTDOWN ");
        }
        else if ((((struct _socket_msg_struct_ *)&recv_buffer)->log_level) == LOG_LEVEL_CRITICAL)
        {
            strcat(msg_to_write, "| Log_Level: LOG_LEVEL_CRITICAL ");
        }

        if ((((struct _socket_msg_struct_ *)&recv_buffer)->log_type) == LOG_TYPE_DATA)
        {
            strcat(msg_to_write, "| Log_Type: LOG_TYPE_DATA ");
        }
        else if ((((struct _socket_msg_struct_ *)&recv_buffer)->log_type) == LOG_TYPE_ERROR)
        {
            strcat(msg_to_write, "| Log_Type: LOG_TYPE_ERROR ");
        }
        else if ((((struct _socket_msg_struct_ *)&recv_buffer)->log_type) == LOG_TYPE_REQUEST)
        {
            strcat(msg_to_write, "| Log_Type: LOG_TYPE_REQUEST ");
        }
        else if ((((struct _socket_msg_struct_ *)&recv_buffer)->log_type) == LOG_TYPE_RESPONSE)
        {
            strcat(msg_to_write, "| Log_Type: LOG_TYPE_RESPONSE ");
        }

        strcat(msg_to_write, " | ");
        strcat(msg_to_write, step_count_msg);

        printf("Message to write: %s\n", msg_to_write);
        int num_written_bytes = write(logger_fd, msg_to_write, strlen(msg_to_write));
    }

}

void sig_handler(int sig_num)
{   
    char buffer[MSG_BUFF_MAX_LEN];
    memset(buffer, '\0', sizeof(buffer));
    
    if (sig_num == SIGINT || sig_num == SIGUSR1)
    {   
        if (sig_num == SIGINT)
            printf("Caught signal %s in logger task\n", "SIGINT");
        else if (sig_num == SIGUSR1)
            printf("Caught signal %s in logger task\n", "SIGKILL");

        g_sig_kill_logger_thread = 1;
        g_sig_kill_sock_hb_thread = 1;

        /* TODO: Add code to flush all the messages to the log file */
        
        //pthread_join(sensor_thread_id, NULL);
        //pthread_join(socket_thread_id, NULL);
        //pthread_join(socket_hb_thread_id, NULL);
        
        mq_close(logger_mq_handle);
        
        exit(0);
    }
}

void logger_task_exit(void)
{
    int mq_close_status = mq_close(logger_mq_handle);
    if (mq_close_status == -1)
        perror("Logger message queue close failed");

    if (logger_fd)
        close(logger_fd);

}
