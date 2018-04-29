#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <termios.h>

#define SOCK_REQ_MSG_API_MSG_LEN             13

#define REQ_RECP_PEDOMETER_TASK              0x1
#define REQ_RECP_HUMIDITY_TASK               0x2

#define TIVA_MSG_TIMESTAMP_ARR_LEN           9

/* UART configuration variables */
struct termios *uart4_config;
int uart4_fd;

struct _ext_app_req_msg_struct_
{
    int req_recipient;
    int params;
    char req_api_msg[SOCK_REQ_MSG_API_MSG_LEN];
};

typedef struct
{
    uint32_t log_level;
    uint32_t log_type;
    uint32_t source_id;
    uint32_t data;
    char timestamp[TIVA_MSG_TIMESTAMP_ARR_LEN];
} sock_msg;

void config_uart_port(struct termios *uart_conf, int uart_desc);

/* UART4 Initialization*/
void uart4_init(void);

int application_init(void);

int test_get_ped_ssid();

int test_get_hum_usid();

int test_get_hum_data();

int test_get_ped_data();

