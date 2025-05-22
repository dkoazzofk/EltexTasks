#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void handlePacket(const char* buffer, int size){
    struct iphdr* iph = (struct iphdr*)buffer;
    struct udphdr* udph = (struct udphdr*)(buffer + iph->ihl * 4);
    if(size < sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(int)){
        printf("Packet too short\n");
        return;
    }

    if(iph->saddr != htonl(0x7f000001) || udph->uh_sport != htons(8080)){
        return;
    }
    printf("\nReceived packet: \n");
    printf("Source IP: %s\n", inet_ntoa(*(struct in_addr*)&iph->saddr));
    printf("Destination IP: %s\n", inet_ntoa(*(struct in_addr*)&iph->daddr));
    printf("Source Port: %d\n", ntohs(udph->source));
    printf("Destination Port: %d\n", ntohs(udph->dest));
    printf("Data Size: %ld bytes\n", size - iph->ihl * 4 - sizeof(struct udphdr) - sizeof(int));

        const char* data = buffer + iph->ihl * 4 + sizeof(struct udphdr) + sizeof(int);
    size_t dataSize = strlen(data);

    if (dataSize > 0) {
        printf("Data: %.*s\n", (int)dataSize, data);

        int fd = open("file.bin", O_CREAT | O_WRONLY | O_APPEND, 0777);
        if (fd == -1) {
            printf("Error: File cannot be opened for writing\n");
            return;
        }

        if (write(fd, data, dataSize) == -1) {
            perror("write");
        }

        close(fd);
    } else {
        printf("No data in packet\n");
    }
}

int main(){
    int sockfd;

    char buffer[];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP) < 0)){
        perror("socket create");
        exit(EXIT_FAILURE);
    }

    while(1){
        int dataSize = recvfrom(sockfd, buffer, (sizeof(struct iphdr) + sizeof(struct udphdr) + 1000) , 0, (struct sockaddr*)&addr, &addr_len);
        if(dataSize <= 0) {
            perror("recf failed");
            break;
        }

        handlePacket(buffer, dataSize);
        memset(buffer, 0, (sizeof(struct iphdr) + sizeof(struct udphdr) + 1000));
    }

    close(sockfd);
    return 0;
}