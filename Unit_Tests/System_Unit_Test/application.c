#include "application.h"

/* UART configuration variables */
char *uart4_port = "/dev/ttyO4";

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
    if((uart4_fd = open(uart4_port, O_RDWR | O_NOCTTY | O_SYNC)) == -1)
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

int application_init(void)
{
    uart4_init();

    return 0;
}

int test_get_ped_ssid()
{
    char recv_buffer[25];
    struct _ext_app_req_msg_struct_ ext_app_req_msg;
    memset(&ext_app_req_msg, 0, sizeof(ext_app_req_msg));

    memset(ext_app_req_msg.req_api_msg, '\0', sizeof(ext_app_req_msg.req_api_msg));
    strncpy(ext_app_req_msg.req_api_msg, "get_ped_ssid", strlen("get_ped_ssid"));
    ext_app_req_msg.req_recipient = REQ_RECP_PEDOMETER_TASK;
    ext_app_req_msg.params = -1;

    if (write(uart4_fd, (char *)&ext_app_req_msg, sizeof(struct _ext_app_req_msg_struct_)) > 0)
    {
        printf("Sent request to Tiva\n");

        if (read(uart4_fd, recv_buffer, sizeof(recv_buffer)) > 0)
        {
            return ((sock_msg *)recv_buffer)->data;
        }
    }
    return -1;
}

int test_get_hum_usid()
{
    char recv_buffer[25];
    struct _ext_app_req_msg_struct_ ext_app_req_msg;
    memset(&ext_app_req_msg, 0, sizeof(ext_app_req_msg));

    memset(ext_app_req_msg.req_api_msg, '\0', sizeof(ext_app_req_msg.req_api_msg));
    strncpy(ext_app_req_msg.req_api_msg, "get_hum_usid", strlen("get_hum_usid"));
    ext_app_req_msg.req_recipient = REQ_RECP_HUMIDITY_TASK;
    ext_app_req_msg.params = -1;

    if (write(uart4_fd, (char *)&ext_app_req_msg, sizeof(struct _ext_app_req_msg_struct_)) > 0)
    {
        printf("Sent request to Tiva\n");

        if (read(uart4_fd, recv_buffer, sizeof(recv_buffer)) > 0)
        {
            return ((sock_msg *)recv_buffer)->data;
        }
    }

    return -1;
}

int test_get_hum_data()
{
    char recv_buffer[25];
    struct _ext_app_req_msg_struct_ ext_app_req_msg;
    memset(&ext_app_req_msg, '\0', sizeof(ext_app_req_msg));

    memset(ext_app_req_msg.req_api_msg, '\0', sizeof(ext_app_req_msg.req_api_msg));
    strncpy(ext_app_req_msg.req_api_msg, "get_hum_data", strlen("get_hum_data"));
    ext_app_req_msg.req_recipient = REQ_RECP_HUMIDITY_TASK;
    ext_app_req_msg.params = -1;

    if (write(uart4_fd, (char *)&ext_app_req_msg, sizeof(struct _ext_app_req_msg_struct_)) > 0)
    {
        printf("Sent request to Tiva\n");

        if (read(uart4_fd, recv_buffer, sizeof(recv_buffer)) > 0)
        {
            if (((sock_msg *)recv_buffer)->data >= 0)
                return 0;
        }
    }
    return -1;
}

int test_get_ped_data()
{
    char recv_buffer[25];
    struct _ext_app_req_msg_struct_ ext_app_req_msg;
    memset(&ext_app_req_msg, '\0', sizeof(ext_app_req_msg));

    memset(ext_app_req_msg.req_api_msg, '\0', sizeof(ext_app_req_msg.req_api_msg));
    strncpy(ext_app_req_msg.req_api_msg, "get_ped_data", strlen("get_ped_data"));
    ext_app_req_msg.req_recipient = REQ_RECP_PEDOMETER_TASK;
    ext_app_req_msg.params = -1;

    if (write(uart4_fd, (char *)&ext_app_req_msg, sizeof(struct _ext_app_req_msg_struct_)) > 0)
    {
        printf("Sent request to Tiva\n");

        if (read(uart4_fd, recv_buffer, sizeof(recv_buffer)) > 0)
        {
            if(((sock_msg *)recv_buffer)->data >= 0)
                return 0;
        }
    }
    return -1;
}
