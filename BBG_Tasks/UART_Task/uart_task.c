/******************************************************************************
 * Author:       Pavan Dhareshwar & Sridhar Pavithrapu
 * Date:         04/18/2018
 * File:         uart_task.c
 * Description:  Source file describing the functionality and implementation
 *               of uart task.
 *****************************************************************************/
#include "uart_task.h"

int main(void)
{
    uart_task_initialized = 0;
	
	/* Initialization of UART4 */
	uart4_init();

    int uart_task_init_status = uart_task_init();
    if (uart_task_init_status)
    {
        printf("Uart task init failed\n");
    }
    else
    {
        printf("Uart task init success\n");
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

    g_sig_kill_uart_rx_thread = 0;
    g_sig_kill_sock_hb_thread = 0;

    pthread_join(uart_rx_thread_id, NULL);
    pthread_join(socket_hb_thread_id, NULL);

    return 0;
}

int uart_task_init(void)
{
    /* We will have all the uart task initializations here */

    /* Set the message queue attributes */
    struct mq_attr logger_mq_attr = { .mq_flags = 0,
        .mq_maxmsg = MSG_QUEUE_MAX_NUM_MSGS,  // Max number of messages on queue
        .mq_msgsize = MSG_QUEUE_MAX_MSG_SIZE  // Max. message size
    };

    logger_mq_handle = mq_open(MSG_QUEUE_NAME, O_RDWR, S_IRWXU, &logger_mq_attr);

    uart_task_initialized = 1;

    return 0;
}

int create_threads(void)
{
    int uart_creat_ret_val = pthread_create(&uart_rx_thread_id, NULL, &uart_rx_thread_func, NULL);
    if (uart_creat_ret_val)
    {
        perror("UART receive thread creation failed");
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

void *uart_rx_thread_func(void *arg)
{
	sock_msg x_sock_data_rcvd;
	
    while(!g_sig_kill_uart_rx_thread)
    {
        memset(&x_sock_data_rcvd,'\0',sizeof(x_sock_data_rcvd));
		
		int received_bytes;
		
		received_bytes = read(uart4_fd, &x_sock_data_rcvd,sizeof(x_sock_data_rcvd));
		if(received_bytes > 0)
		{
			printf("\nReceived data from TIVA:\n");
			printf("Log Level:%d\n", x_sock_data_rcvd.log_level);
			printf("Log Type:%d\n", x_sock_data_rcvd.log_type);
			printf("Source ID:%d\n", x_sock_data_rcvd.source_id);
			printf("Step count is:%d\n",x_sock_data_rcvd.data);
		
            post_data_to_logger_queue(x_sock_data_rcvd);
        }
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
            if (uart_task_initialized == 1)
                strcpy(send_buffer, "Initialized");
            else
                strcpy(send_buffer, "Uninitialized");

            ssize_t num_sent_bytes = send(accept_conn_id, send_buffer, strlen(send_buffer), 0);
            if (num_sent_bytes < 0)
                perror("send failed");
        }
    }
}

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

void post_data_to_logger_queue(sock_msg x_sock_data)
{
    int msg_priority = 1;

    int num_sent_bytes = mq_send(logger_mq_handle, (char *)&x_sock_data,
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
        
        g_sig_kill_uart_rx_thread = 1;
        g_sig_kill_sock_hb_thread = 1;
        
        //pthread_join(sensor_thread_id, NULL);
        //pthread_join(uart_rx_thread_id, NULL);
        //pthread_join(socket_hb_thread_id, NULL);
        
        mq_close(logger_mq_handle);
        
        exit(0);
    }
}
