#include "structs.h"


int main() {
    int cli_mq = create_msg_queue(CLI_QUEUE_KEY);
    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[2];
    
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
    
    ev.data.fd = cli_mq;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cli_mq, &ev);
    
    printf("Taxi Management CLI\n");
    printf("Commands: create_driver, send_task <pid> <timer>, get_status <pid>, get_drivers\n");
    
    while (1) {
        printf(">> ");
        fflush(stdout);
        
        int nfds = epoll_wait(epoll_fd, events, 2, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == cli_mq) {
                driver_msg msg;
                while (recv_message(cli_mq, &msg, 1) == 0) {
                    switch (msg.type) {
                        case REGISTER:
                            printf("Received REGISTER for PID %d\n", msg.pid);
                            if (num_drivers < MAX_DRIVERS) {
                                drivers[num_drivers].pid = msg.pid;
                                drivers[num_drivers].message_queue_id = create_msg_queue(msg.data);
                                drivers[num_drivers].status = AVAILABLE;
                                drivers[num_drivers].remaining_time = 0;
                                num_drivers++;
                                printf("Driver %d registered\n", msg.pid);
                            }
                            break;
                        case STATUS_UPDATE:
                            update_driver_status(msg.pid, msg.data > 0, msg.data);
                            break;
                        case BUSY_RESP:
                            printf("Driver %d is busy (%d sec left)\n", 
                                   msg.pid, msg.data);
                            break;
                    }
                }
            }
            if (events[i].data.fd == STDIN_FILENO) {
                char cmd[50];
                if(fgets(cmd, sizeof(cmd), stdin)) {
                    cmd[strcspn(cmd, "\n")] = 0;
                    
                    if(strcmp(cmd, "create_driver") == 0) {
                        create_driver();
                    }
                    else if(strcmp(cmd, "get_drivers") == 0) {
                        get_drivers();
                    }
                    else if(strncmp(cmd, "send_task", 9) == 0) {
                        pid_t pid;
                        int timer;
                        if (sscanf(cmd + 10, "%d %d", &pid, &timer) == 2) {
                            send_task(pid, timer);
                        } else {
                            printf("Usage: send_task <pid> <timer>\n");
                        }
                    }
                    else if(strncmp(cmd, "get_status", 10) == 0) {
                        pid_t pid;
                        if (sscanf(cmd + 11, "%d", &pid) == 1) {
                            get_status(pid);
                        } else {
                            printf("Usage: get_status <pid>\n");
                        }
                    }
                    else {
                        printf("Unknown command\n");
                    }
                }
            }
        }
    }
    return 0;
}