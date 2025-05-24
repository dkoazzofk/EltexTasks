#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


int calculate(char *buff, int newsockfd);

double sum(double a, double b) {
    return a + b;
}

double division(double a, double b) {
    if (b == 0)
        return 0;
    return a / b;
}

double multiplication(double a, double b) {
    return a * b;
}

double subtraction(double a, double b) {
    return a - b;
}

void close_sock(fd_set fds, int max_fd) {
    for (size_t i = 3; i <= max_fd + 1; i++) {
        if (FD_ISSET(i, &fds)) {
            close(i);
        }
    }
}

typedef struct
{
    const char *name;
    double (*func)(double, double);
} Operation;

int main(int argc, char *argv[]) {
    int sockfd;
    int newsockfd;
    pid_t pid;
    struct sockaddr_in servaddr, cliaddr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("sockfd");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (listen(sockfd, 5) < 0) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    fd_set fds;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&fds);
    int res;
    char buff[1024];
    FD_SET(sockfd, &fds);
    int max_fd = sockfd;
    int sel_fds;
    struct timeval delay;
    socklen_t clilen = sizeof(cliaddr);

    for (;;) {
        delay.tv_sec = 60;
        delay.tv_usec = 0;
        read_fds = fds;
        if ((sel_fds = select(max_fd + 1, &read_fds, 0, 0, &delay)) < 0) {
            perror("select");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        if (sel_fds == 0) {
            close_sock(fds, max_fd);
            break;
        }
        for (size_t i = 3; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == sockfd) {
                    if ((newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                        perror("accept");
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(newsockfd, &fds);
                    max_fd = (newsockfd > max_fd) ? newsockfd : max_fd;
                    printf("IP %s connected\n", inet_ntoa(cliaddr.sin_addr));
                    strncpy(buff, "Enter a +-*/ b\n", 1024);
                    if (write(newsockfd, buff, strlen(buff) + 1) < 0) {
                        perror("write");
                        close(newsockfd);
                        exit(EXIT_FAILURE);
                    }
                } else {
                    if ((res = read(i, buff, sizeof(buff))) < 1) {
                        if (res == 0) {
                            FD_CLR(i, &fds);
                            close(i);
                            continue;
                        }
                        perror("read");
                        close_sock(fds, max_fd);
                        exit(EXIT_FAILURE);
                    }
                    if (calculate(buff, i) == EXIT_FAILURE) {
                        close_sock(fds, max_fd);
                        exit(EXIT_FAILURE);
                    }
                    sleep(1);
                    strncpy(buff, "Enter a +-*/ b\n", 1024);
                    if (write(i, buff, strlen(buff) + 1) < 0) {
                        perror("write");
                        close_sock(fds, max_fd);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
    exit(EXIT_SUCCESS);
}


int calculate(char *buff, int newsockfd) {
    Operation operation[] = {
        {"+", sum},
        {"/", division},
        {"*", multiplication},
        {"-", subtraction},
    };
    double a, b, result;
    char temp[2];

    if (sscanf(buff, "%lf %s %lf", &a, temp, &b) < 3) {
        strncpy(buff, "Incorrect data\n", 1024);
        if (write(newsockfd, buff, strlen(buff) + 1) < 0) {
            perror("write");
            close(newsockfd);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    int i = 0;
    while (i < sizeof(operation) / sizeof(operation[0])) {
        if (temp[0] == operation[i].name[0]) {
            result = operation[i].func(a, b);
            break;
        }
        i++;
    }

    if (result == 0 && operation[i].name[0] == '/') {
        strncpy(buff, "You can't divide by zero\n", 1024);
        if (write(newsockfd, buff, strlen(buff) + 1) < 0) {
            perror("write");
            close(newsockfd);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    snprintf(buff, sizeof(buff), "%.2lf", result);
    if (write(newsockfd, buff, strlen(buff) + 1) < 0) {
        perror("write");
        close(newsockfd);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}