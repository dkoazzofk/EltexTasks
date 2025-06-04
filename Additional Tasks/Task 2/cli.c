#include "structs.h"

DriverInfo drivers[MAX_DRIVERS];
int num_drivers = 0;

int create_msg_queue(key_t key){
    int mq_id = msgget(key, IPC_CREAT | 0666);
    if(mq_id < 0){
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    return mq_id;
}

void send_message(int mq_id, driver_msg *msg){
    if(msgsnd(mq_id, msg, sizeof(driver_msg) - sizeof(long), 0) < 0){
        perror("msgsend");
    } else {
        if (msg->type == REGISTER) {
            printf("Driver %d sent REGISTER message\n", msg->pid);
        }
    }
}

int recv_message(int mq_id, driver_msg *msg, int non_block){
    int flags = non_block ? IPC_NOWAIT : 0;
    ssize_t ret = msgrcv(mq_id, msg, sizeof(driver_msg) - sizeof(long), 0, flags);
    if(ret < 0){
        if(errno != ENOMSG && !non_block) perror("msgrcv");
        return -1;
    }
    return 0;
}

void create_driver() {
    pid_t pid = fork();
    if(pid == 0) {
        key_t key = BASE_DRIVER_KEY + getpid();
        int mq_id = create_msg_queue(key);
        
        driver_msg reg_msg = {
            .mtype = 1,
            .type = REGISTER,
            .pid = getpid(),
            .data = key
        };
        
        int cli_mq = create_msg_queue(CLI_QUEUE_KEY);
        send_message(cli_mq, &reg_msg);
        
        int epoll_fd = epoll_create1(0);
        struct epoll_event ev, events[2];
        
        ev.events = EPOLLIN;
        ev.data.fd = mq_id;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, mq_id, &ev);
        
        int busy = 0;
        int remaining = 0;
        int timer_fd = -1;
        
        while (1) {
            int nfds = epoll_wait(epoll_fd, events, 2, -1);
            for(int i = 0; i < nfds; i++) {
                if(events[i].data.fd == mq_id) {
                    driver_msg msg;
                    if(recv_message(mq_id, &msg, 1) == 0) {
                        if(msg.type == TASK) {
                            if(!busy) {
                                busy = 1;
                                remaining = msg.data;
                                timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
                                struct itimerspec timer_spec = {
                                    .it_value = {.tv_sec = msg.data, .tv_nsec = 0},
                                    .it_interval = {0, 0}
                                };
                                timerfd_settime(timer_fd, 0, &timer_spec, NULL);
                                
                                ev.events = EPOLLIN;
                                ev.data.fd = timer_fd;
                                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev);
                                
                                driver_msg update = {
                                    .mtype = 1,
                                    .type = STATUS_UPDATE,
                                    .pid = getpid(),
                                    .data = remaining
                                };
                                send_message(cli_mq, &update);
                            } else {
                                driver_msg busy_msg = {
                                    .mtype = 1,
                                    .type = BUSY_RESP,
                                    .pid = getpid(),
                                    .data = remaining
                                };
                                send_message(cli_mq, &busy_msg);
                            }
                        }
                    }
                } else if(timer_fd != -1 && events[i].data.fd == timer_fd) {
                    uint64_t exp;
                    read(timer_fd, &exp, sizeof(exp));
                    busy = 0;
                    remaining = 0;
                    close(timer_fd);
                    timer_fd = -1;
                    
                    driver_msg update = {
                        .mtype = 1,
                        .type = STATUS_UPDATE,
                        .pid = getpid(),
                        .data = 0
                    };
                    send_message(cli_mq, &update);
                }
            }
        }
    } else if(pid > 0) {
        printf("Driver created with PID: %d\n", pid);
    } else {
        perror("fork failed");
    }
}

int find_driver(pid_t pid) {
    for(int i = 0; i < num_drivers; i++) {
        if (drivers[i].pid == pid) return i;
    }
    return -1;
}

void update_driver_status(pid_t pid, int busy, int remaining) {
    int idx = find_driver(pid);
    if(idx >= 0) {
        drivers[idx].status = busy ? BUSY : AVAILABLE;
        drivers[idx].remaining_time = remaining;
    }
}

void send_task(pid_t pid, int timer) {
    int idx = find_driver(pid);
    if(idx < 0) {
        printf("Driver %d not found\n", pid);
        return;
    }
    
    driver_msg msg = {
        .mtype = 1,
        .type = TASK,
        .pid = getpid(),
        .data = timer
    };
    send_message(drivers[idx].message_queue_id, &msg);
}

void get_status(pid_t pid) {
    int idx = find_driver(pid);
    if(idx < 0) {
        printf("Driver %d not found\n", pid);
        return;
    }
    
    printf("Driver %d status: %s", pid, 
           drivers[idx].status == BUSY ? "Busy" : "Available");
    if(drivers[idx].status == BUSY) {
        printf(" (%d sec left)\n", drivers[idx].remaining_time);
    } else {
        printf("\n");
    }
}

void get_drivers() {
    for(int i = 0; i < num_drivers; i++) {
        printf("Driver PID: %d, Status: %s", 
               drivers[i].pid,
               drivers[i].status == BUSY ? "Busy" : "Available");
        if(drivers[i].status == BUSY) {
            printf(" (%d sec left)\n", drivers[i].remaining_time);
        } else {
            printf("\n");
        }
    }
}