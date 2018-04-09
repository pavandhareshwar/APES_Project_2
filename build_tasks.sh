#!/bin/sh

rm Main_Task/main_task
rm Logger_Task/logger_task

gcc Logger_Task/logger_task.c -o Logger_Task/logger_task -lrt -lpthread

#gcc socket_task.c -o socket_task -lrt -lpthread

#gcc decision_task.c -o decision_task -lrt -lpthread

gcc Main_Task/main_task.c -o Main_Task/main_task -lrt -lpthread
