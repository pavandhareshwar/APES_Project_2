/******************************************************************************
 * Author:       Pavan Dhareshwar & Sridhar Pavithrapu
 * Date:         04/08/2018
 * File:         decision_task.c
 * Description:  Source file describing the functionality and implementation
 *               of decision task.
 *****************************************************************************/
#include "decision_task.h"

int main(void)
{
    decision_task_initialized = 0;

    int decision_task_init_status = decision_task_init();
    if (decision_task_init_status)
    {
        printf("Decision task init failed\n");
    }
    else
    {
        printf("Decision task init success\n");
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

    g_sig_kill_decision_thread = 0;
    g_sig_kill_sock_hb_thread = 0;

    pthread_join(decision_thread_id, NULL);
    pthread_join(socket_hb_thread_id, NULL);

    return 0;
}

int decision_task_init(void)
{
    /* Create the logger task message queue handle */
    /* Set the message queue attributes */
    struct mq_attr logger_mq_attr = { .mq_flags = 0,
                                        .mq_maxmsg = MSG_QUEUE_MAX_NUM_MSGS,  // Max number of messages on queue
                                        .mq_msgsize = MSG_QUEUE_MAX_MSG_SIZE  // Max. message size
                                    };

    logger_mq_handle = mq_open(LOGGER_MSG_QUEUE_NAME, O_RDWR, S_IRWXU, &logger_mq_attr);

    /* Create a message queue that will be shared between uart task and the decision task to
     * share the message received from Tiva */
    struct mq_attr decision_mq_attr = { .mq_flags = 0,
                                      .mq_maxmsg = MSG_QUEUE_MAX_NUM_MSGS,  // Max number of messages on queue
                                      .mq_msgsize = MSG_QUEUE_MAX_MSG_SIZE  // Max. message size
                                    };


    decision_mq_handle = mq_open(DECISION_MSG_QUEUE_NAME, O_CREAT | O_RDWR, S_IRWXU, &decision_mq_attr);
    if (decision_mq_handle < 0)
    {
        perror("Decision task message queue create failed");
        return -1;
    }

    printf("Decision task message queue successfully created\n")

    decision_task_initialized = 1;

    return 0;
}

int create_threads(void)
{
    int decision_t_creat_ret_val = pthread_create(&decision_thread_id, NULL, &decision_thread_func, NULL);
    if (decision_t_creat_ret_val)
    {
        perror("Decision thread creation failed");
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

void *decision_thread_func(void *arg)
{
    while(!g_sig_kill_decision_thread)
    {
        /* This thread will continuously wait for messages on the message queue
         * and process it to take some decision */
        read_from_decision_task_msg_queue();
    
        sleep(5);
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
            /* For the sake of start-up check, because we have the decision task initialized
            ** by the time this thread is spawned, we send a response that the task is initialized */
            if (decision_task_initialized == 1)
                strcpy(send_buffer, "Initialized");
            else
                strcpy(send_buffer, "Uninitialized");

            ssize_t num_sent_bytes = send(accept_conn_id, send_buffer, strlen(send_buffer), 0);
            if (num_sent_bytes < 0)
                perror("send failed");
        }
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

void read_from_decision_task_msg_queue(void)
{
    char recv_buffer[MSG_MAX_LEN];
    memset(recv_buffer, '\0', sizeof(recv_buffer));

    int msg_priority;

    int num_recv_bytes;
    while ((num_recv_bytes = mq_receive(decision_mq_handle, (char *)&recv_buffer,
                    MSG_QUEUE_MAX_MSG_SIZE, &msg_priority)) != -1)
    {
        if (num_recv_bytes < 0)
        {
            perror("mq_receive failed");
            return;
        }

#if 0
        printf("Pedometer count: %d, msg_src: %d, log level: %d, log_type: %d\n",
            (((struct _socket_msg_struct_ *)&recv_buffer)->data),
            (((struct _socket_msg_struct_ *)&recv_buffer)->source_id),
            (((struct _socket_msg_struct_ *)&recv_buffer)->log_level),
            (((struct _socket_msg_struct_ *)&recv_buffer)->log_type));
#endif
    }
}

void sig_handler(int sig_num)
{
    char buffer[MSG_BUFF_MAX_LEN];
    memset(buffer, '\0', sizeof(buffer));
    
    if (sig_num == SIGINT || sig_num == SIGUSR1)
    {   
        if (sig_num == SIGINT)
            printf("Caught signal %s in decision task\n", "SIGINT");
        else if (sig_num == SIGUSR1)
            printf("Caught signal %s in decision task\n", "SIGKILL");
        
        g_sig_kill_decision_thread = 1;
        g_sig_kill_sock_hb_thread = 1;
        
        //pthread_join(sensor_thread_id, NULL);
        //pthread_join(socket_thread_id, NULL);
        //pthread_join(socket_hb_thread_id, NULL);
        
        mq_close(logger_mq_handle);
        
        exit(0);
    }
}
