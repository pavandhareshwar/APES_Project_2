#!/bin/sh

rm Main_Task/main_task
rm Logger_Task/logger_task
rm Decision_Task/decision_task
rm Socket_Task/socket_task
rm UART_Task/uart_task

gcc Main_Task/main_task.c -o Main_Task/main_task -lrt -lpthread

gcc Logger_Task/logger_task.c -o Logger_Task/logger_task -lrt -lpthread

gcc Socket_Task/socket_task.c -o Socket_Task/socket_task -lrt -lpthread

gcc Decision_Task/decision_task.c -o Decision_Task/decision_task -lrt -lpthread

gcc UART_Task/uart_task.c -o UART_Task/uart_task -lrt -lpthread
