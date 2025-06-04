#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/epoll.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/timerfd.h>

#define MAX_DRIVERS 50
#define MAX_BUFFER 128
#define CLI_QUEUE_KEY 0x1234
#define BASE_DRIVER_KEY 0x5678 

typedef enum {
    AVAILABLE,
    BUSY
} DriverStatus;

typedef enum {
    REGISTER = 1,
    TASK,
    STATUS_REQ,
    STATUS_RESP,
    STATUS_UPDATE,
    BUSY_RESP
} MessageType;

typedef struct {
    long mtype;
    MessageType type;
    pid_t pid;
    int data;
    char message[50];
} driver_msg;

typedef struct {
    pid_t pid;
    DriverStatus status;
    int remaining_time;
    int message_queue_id;
    int timer_fd;
} DriverInfo;

extern DriverInfo drivers[MAX_DRIVERS];
extern int num_drivers;

int create_msg_queue(key_t key);
void send_message(int mq_id, driver_msg *msg);
int recv_message(int mq_id, driver_msg *msg, int non_block);
void create_driver();
int find_driver(pid_t pid);
void update_driver_status(pid_t pid, int busy, int remaining);
void send_task(pid_t pid, int timer);
void get_status(pid_t pid);
void get_drivers();

#endif