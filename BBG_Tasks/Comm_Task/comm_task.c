/******************************************************************************
 * Author:       Pavan Dhareshwar & Sridhar Pavithrapu
 * Date:         04/08/2018
 * File:         decision_task.c
 * Description:  Source file describing the functionality and implementation
 *               of decision task.
 *****************************************************************************/
#include "comm_task.h"

int main(void)
{
    comm_task_initialized = 0;

    int comm_task_init_status = comm_task_init();
    if (comm_task_init_status)
    {
        printf("Comm task init failed\n");
    }
    else
    {
        printf("Comm task init success\n");
    }

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

    g_sig_kill_comm_thread = 0;
    g_sig_kill_sock_hb_thread = 0;
    gb_waiting_for_ext_app_uart_rsp = false;

    pthread_join(comm_thread_id, NULL);
    pthread_join(socket_hb_thread_id, NULL);

    return 0;
}

int comm_task_init(void)
{
    /* We will have all the socket task initializations here */
    /* Create the logger task message queue handle */
    /* Set the message queue attributes */
    struct mq_attr logger_mq_attr = { .mq_flags = 0,
                                        .mq_maxmsg = MSG_QUEUE_MAX_NUM_MSGS,  // Max number of messages on queue
                                        .mq_msgsize = MSG_QUEUE_MAX_MSG_SIZE  // Max. message size
                                    };

    logger_mq_handle = mq_open(LOGGER_MSG_QUEUE_NAME, O_RDWR, S_IRWXU, &logger_mq_attr);

    /* Set the decision message queue attributes */
     struct mq_attr decision_mq_attr = { .mq_flags = 0,
                                         .mq_maxmsg = MSG_QUEUE_MAX_NUM_MSGS,  // Max number of messages on queue
                                         .mq_msgsize = MSG_QUEUE_MAX_MSG_SIZE  // Max. message size
                                        };
 
     decision_mq_handle = mq_open(DECISION_MSG_QUEUE_NAME, O_RDWR, S_IRWXU, &decision_mq_attr);

#ifdef USE_UART_FOR_COMM
    uart4_init();

    /* Not initializing the semaphore with 1 because we want the external app 
     * socket interface thread to wait on this semaphore, which will be posted
     * in communication thread 
     */
    sem_init(&sem_ext_app_req_rsp, 0, 0);
    sem_init(&sem_sock_msg_shared, 0, 1);

#endif

    /* Creating a socket that is exposed to the external application */
    initialize_server_socket(&server_addr, SERVER_PORT_NUM, SERVER_LISTEN_QUEUE_SIZE);

    comm_task_initialized = 1;

    return 0;
}

int create_threads(void)
{
    int comm_t_creat_ret_val = pthread_create(&comm_thread_id, NULL, &comm_thread_func, NULL);
    if (comm_t_creat_ret_val)
    {
        perror("Comm thread creation failed");
        return -1;
    }

    int sock_hb_t_creat_ret_val = pthread_create(&socket_hb_thread_id, NULL, &socket_hb_thread_func, NULL);
    if (sock_hb_t_creat_ret_val)
    {
        perror("Socket heartbeat thread creation failed");
        return -1;
    }

    int ext_app_int_t_creat_ret_val = pthread_create(&ext_app_int_thread_id, NULL, &ext_app_int_thread_func, NULL);
    if (ext_app_int_t_creat_ret_val)
    {
        perror("External Application socket interface thread creation failed\n");
        return -1;
    }

    return 0;
}

void *comm_thread_func(void *arg)
{
    char ext_app_rsp_msg[32];
    size_t sent_bytes;
    sock_msg x_sock_data_rcvd;
    char recv_buffer[25];

    while(!g_sig_kill_comm_thread)
    {
        //memset(&x_sock_data_rcvd,'\0',sizeof(x_sock_data_rcvd));
        memset(recv_buffer,'\0',sizeof(recv_buffer));

#ifdef USE_UART_FOR_COMM
        //if (read(uart4_fd, &x_sock_data_rcvd, sizeof(x_sock_data_rcvd)) > 0)
        if (read(uart4_fd, recv_buffer, sizeof(recv_buffer)) > 0)
        {
#if 1
            printf("\nReceived data from TIVA:\n");
            //printf("Log Level:%d\n", x_sock_data_rcvd.log_level);
            //printf("Log Type:%d\n", x_sock_data_rcvd.log_type);
            //printf("Source ID:%d\n", x_sock_data_rcvd.source_id);
            printf("Log Level:%d\n", ((sock_msg *)recv_buffer)->log_level);
            printf("Log Type:%d\n", ((sock_msg *)recv_buffer)->log_type);
            printf("Source ID:%d\n", ((sock_msg *)recv_buffer)->source_id);
            if (x_sock_data_rcvd.source_id == TASK_PEDOMETER)
            {
                //printf("Step count is:%d\n",x_sock_data_rcvd.data);
                printf("Step count is:%d\n", ((sock_msg *)recv_buffer)->data);
            }
            else
            {
                //printf("Humidity data is:%d\n",x_sock_data_rcvd.data);
                printf("Humidity data is:%d\n", ((sock_msg *)recv_buffer)->data);
            }
            printf("TimeStamp: %s\n", ((sock_msg *)recv_buffer)->timestamp);
            //printf("TimeStamp: %s\n", x_sock_data_rcvd.timestamp);

            x_sock_data_rcvd.log_level = ((sock_msg *)recv_buffer)->log_level;
            x_sock_data_rcvd.log_type = ((sock_msg *)recv_buffer)->log_type;
            x_sock_data_rcvd.source_id = ((sock_msg *)recv_buffer)->source_id;
            x_sock_data_rcvd.data = ((sock_msg *)recv_buffer)->data;
            strcpy(x_sock_data_rcvd.timestamp, ((sock_msg *)recv_buffer)->timestamp);

#endif  
            if (gb_waiting_for_ext_app_uart_rsp == true)
            {
                gb_waiting_for_ext_app_uart_rsp = false;
          
                sem_wait(&sem_sock_msg_shared);
                memcpy(&x_sock_data_rcvd_shared, &x_sock_data_rcvd, sizeof(sock_msg));            
                sem_post(&sem_sock_msg_shared);

                sem_post(&sem_ext_app_req_rsp);
            }
            else
            {
                post_data_to_logger_task_queue(x_sock_data_rcvd);
            
                post_data_to_decision_task_queue(x_sock_data_rcvd);
            }
        }

#endif
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
            /* For the sake of start-up check, because we have the main socket task 
             * initialized by the time this thread is spawned, we send a response 
             * that the task is initialized */
            if (comm_task_initialized == 1)
                strcpy(send_buffer, "Initialized");
            else
                strcpy(send_buffer, "Uninitialized");

            ssize_t num_sent_bytes = send(accept_conn_id, send_buffer, strlen(send_buffer), 0);
            if (num_sent_bytes < 0)
                perror("send failed");
        }
    }
}

void *ext_app_int_thread_func(void *arg)
{
    int serv_addr_len = sizeof(server_addr);
	char buffer[BUFF_SIZE];
    size_t sent_bytes;
    
    /* Wait for request from external application */
    if ((accept_conn_id = accept(server_sockfd, (struct sockaddr *)&server_addr, 
                    (socklen_t *)&serv_addr_len)) < 0)
    {
        perror("accept");
    }

    char recv_buffer[BUFF_SIZE];
	while(!g_sig_kill_ext_app_sock_int_thread)
    {
        memset(recv_buffer, '\0', sizeof(recv_buffer));
        int num_recv_bytes = recv(accept_conn_id, recv_buffer, sizeof(recv_buffer), 0);
        if (num_recv_bytes < 0)
        {
            printf("recv failed in socket task\n");
            perror("recv failed");
        }
        else
        {
            /* Handle external application request here */
            if (*(((struct _ext_app_req_msg_struct_ *)&recv_buffer)->req_api_msg) != '\0')
            {
                printf("Message req api: %s, req recp: %s, req api params: %d\n",
                        (((struct _ext_app_req_msg_struct_ *)&recv_buffer)->req_api_msg),
                        (((((struct _ext_app_req_msg_struct_ *)&recv_buffer)->req_recipient) 
                         == TASK_PEDOMETER) ? "Pedometer Task" : "Humidity Task"),
                        (((struct _ext_app_req_msg_struct_ *)&recv_buffer)->params));

                // strncpy(buffer, "Test Response!", strlen("Test Response!"));
                // sent_bytes = send(accept_conn_id, buffer, strlen(buffer), 0);
                
                if ((((struct _ext_app_req_msg_struct_ *)&recv_buffer)->req_recipient) == TASK_PEDOMETER)
                {
                    /* We will call a UART_Send here to send the external application request
                     * to Tiva and set the flag gb_waiting_for_ext_app_uart_rsp to true to
                     * make the comm_thread wait for response */
                
                    if (write(uart4_fd, (char *)&recv_buffer, sizeof(struct _ext_app_req_msg_struct_)) > 0)
                    {
                        printf("Sent request to Tiva\n");
                        gb_waiting_for_ext_app_uart_rsp = true;
                    
                        sem_wait(&sem_ext_app_req_rsp);

                        char rsp_buffer[32];
                        memset(rsp_buffer, '\0', sizeof(rsp_buffer));

                        sem_wait(&sem_sock_msg_shared);
                        sprintf(rsp_buffer, "Step count: %d\n", x_sock_data_rcvd_shared.data);
                        sem_post(&sem_sock_msg_shared);

                        size_t sent_bytes = send(accept_conn_id, rsp_buffer, strlen(rsp_buffer), 0);
                        if (sent_bytes <= 0)
                        {
                            perror("send failed in comm_task to ext_app\n");
                        }
                    }
                }
                else if ((((struct _ext_app_req_msg_struct_ *)&recv_buffer)->req_recipient) == TASK_HUMIDITY)
                {
                    if (write(uart4_fd, (char *)&recv_buffer, sizeof(struct _ext_app_req_msg_struct_)) > 0)
                    {
                        printf("Sent request to Tiva\n");
                        gb_waiting_for_ext_app_uart_rsp = true;
                        
                        sem_wait(&sem_ext_app_req_rsp);

                        char rsp_buffer[32];
                        memset(rsp_buffer, '\0', sizeof(rsp_buffer));

                        sprintf(rsp_buffer, "Humidity data: %d\n", ((sock_msg *)recv_buffer)->data);

                        size_t sent_bytes = send(accept_conn_id, buffer, strlen(buffer), 0);
                        if (sent_bytes <= 0)
                        {
                            perror("send failed in comm_task to ext_app\n");
                        }
                    }

                }
            }
        }
    }

    pthread_exit(NULL);
}

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

#ifdef USE_UART_FOR_COMM

/* UART4 Configuration */
void config_uart_port(struct termios *uart_conf, int uart_desc)
{
    tcgetattr(uart_desc, uart_conf);

    uart_conf->c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    uart_conf->c_oflag = 0;
    uart_conf->c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    uart_conf->c_cc[VMIN] = 1;
    uart_conf->c_cc[VTIME] = 0;

    if(cfsetispeed(uart_conf, B115200) || cfsetospeed(uart_conf, B115200))
    {
        perror("Error in setting baud rate for UART4\n");
    }


    if(tcsetattr(uart_desc, TCSAFLUSH, uart_conf) < 0)
    {
        perror("Error in setting attributes for UART4\n");
    }
}

/* UART4 Initialization*/
void uart4_init(void)
{
    if((uart4_fd = open(uart4_port, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK)) == -1)
    {
        perror("Error in opening UART4 file descriptor\n");
        return;
    }

    uart4_config = (struct termios*)malloc(sizeof(struct termios));
    config_uart_port(uart4_config, uart4_fd);

    if(tcsetattr(uart4_fd,TCSAFLUSH, uart4_config) < 0)
    {
        printf("Error in setting attributes for UART port\n");
        return;
    }
}
#endif

void initialize_server_socket(struct sockaddr_in *sock_addr_struct, 
                                int port_num, int listen_queue_size)
{
	/* Create server socket */ 
	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}	

    int option = 1;
    if(setsockopt(server_sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR),
                    (char*)&option,sizeof(option)) < 0)
    {
        printf("setsockopt failed\n");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    sock_addr_struct->sin_family = AF_INET;
	sock_addr_struct->sin_addr.s_addr = INADDR_ANY;
	sock_addr_struct->sin_port = htons(port_num);

	if (bind(server_sockfd, (struct sockaddr *)sock_addr_struct, 
							sizeof(struct sockaddr_in))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_sockfd, listen_queue_size) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
}

void post_data_to_logger_task_queue(sock_msg x_sock_data)
{
    int msg_priority = 1;

    int num_sent_bytes = mq_send(logger_mq_handle, (char *)&x_sock_data,
            sizeof(sock_msg), msg_priority);
    if (num_sent_bytes < 0)
        perror("mq_send failed");
}

void post_data_to_decision_task_queue(sock_msg x_sock_data)
{
    int msg_priority = 1;

    int num_sent_bytes = mq_send(decision_mq_handle, (char *)&x_sock_data,
            sizeof(sock_msg), msg_priority);
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
            printf("Caught signal %s in socket task\n", "SIGINT");
        else if (sig_num == SIGUSR1)
            printf("Caught signal %s in socket task\n", "SIGKILL");
        
        g_sig_kill_comm_thread = 1;
        g_sig_kill_sock_hb_thread = 1;
        
        //pthread_join(sensor_thread_id, NULL);
        //pthread_join(socket_thread_id, NULL);
        //pthread_join(socket_hb_thread_id, NULL);
        
        mq_close(logger_mq_handle);
        
        exit(0);
    }
}
