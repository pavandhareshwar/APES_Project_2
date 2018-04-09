#!/bin/sh

rm Main_Task/main_task

#gcc logger_task.c -o logger_task -lrt -lpthread

#gcc socket_task.c -o socket_task -lrt -lpthread

#gcc decision_task.c -o decision_task -lrt -lpthread

gcc Main_Task/main_task.c -o Main_Task/main_task -lrt -lpthread
