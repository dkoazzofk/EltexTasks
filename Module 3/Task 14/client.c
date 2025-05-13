#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in clientvaddr;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket create");
        exit(EXIT_FAILURE);
    }

    memset(&clientvaddr, 0, sizeof(clientvaddr));

    clientvaddr.sin_family = AF_INET;
    clientvaddr.sin_port = htons(PORT);
    clientvaddr.sin_addr.s_addr = INADDR_ANY;

    socklen_t len = sizeof(clientvaddr);

    while(1){
        printf("Clien: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&clientvaddr, len);

        if(strcmp(buffer, "exit\n") == 0){
            printf("Client end chat.");
            break;
        }
        int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&clientvaddr, &len);
        buffer[n] = '\0';
        printf("Server: %s", buffer);
    }

    close(sockfd);
    return 0;
}