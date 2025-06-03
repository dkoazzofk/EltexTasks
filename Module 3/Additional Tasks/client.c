#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/udp.h>
#include <netinet/ip.h>

#define BUFFER_SIZE 1024
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 9090
#define DEFAULT_CLIENT_PORT 54321

int client_running = 1;
char server_ip[16] = DEFAULT_SERVER_IP;
unsigned short server_port = DEFAULT_SERVER_PORT;
unsigned short client_port = DEFAULT_CLIENT_PORT;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        client_running = 0;
    }
}

unsigned short csum(unsigned short *buf, int nwords) {
    unsigned long sum = 0;
    for (; nwords > 0; nwords--) {
        sum += *buf++;
        sum = (sum >> 16) + (sum & 0xffff);
    }
    return (unsigned short)(~sum);
}

unsigned short udp_checksum(struct iphdr *iph, struct udphdr *udph, char *data, int data_len) {
    struct pseudohdr {
        unsigned int saddr;
        unsigned int daddr;
        unsigned char zero;
        unsigned char protocol;
        unsigned short length;
    } ph;
    
    ph.saddr = iph->saddr;
    ph.daddr = iph->daddr;
    ph.zero = 0;
    ph.protocol = IPPROTO_UDP;
    ph.length = udph->len;
    
    int total_len = sizeof(ph) + sizeof(struct udphdr) + data_len;
    char *buf = (char *)malloc(total_len);
    if (!buf) {
        perror("malloc");
        return 0;
    }
    
    memcpy(buf, &ph, sizeof(ph));
    memcpy(buf + sizeof(ph), udph, sizeof(struct udphdr));
    memcpy(buf + sizeof(ph) + sizeof(struct udphdr), data, data_len);
    
    unsigned short checksum = csum((unsigned short *)buf, (total_len + 1) / 2);
    free(buf);
    return checksum;
}

void send_close_message(int sockfd) {
    char packet[BUFFER_SIZE];
    memset(packet, 0, BUFFER_SIZE);


    struct iphdr *iph = (struct iphdr *)packet;
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen("CLOSE_CONNECTION") + 1);
    iph->id = htons(12345);
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;
    iph->saddr = inet_addr("127.0.0.1");
    iph->daddr = inet_addr(server_ip);
    iph->check = 0;
    iph->check = csum((unsigned short *)iph, iph->ihl * 2);

    struct udphdr *udph = (struct udphdr *)(packet + sizeof(struct iphdr));
    udph->source = htons(client_port);
    udph->dest = htons(server_port);
    udph->len = htons(sizeof(struct udphdr) + strlen("CLOSE_CONNECTION") + 1);
    udph->check = 0;

    char *data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(data, "CLOSE_CONNECTION", strlen("CLOSE_CONNECTION") + 1);

    udph->check = udp_checksum(iph, udph, data, strlen("CLOSE_CONNECTION") + 1);

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = iph->daddr;
    dest_addr.sin_port = htons(server_port);
    
    if (sendto(sockfd, packet, ntohs(iph->tot_len), 0, 
               (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
    } else {
        printf("Sent close message to %s:%d\n", 
               inet_ntoa(dest_addr.sin_addr), ntohs(dest_addr.sin_port));
    }
}

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in saddr;
    socklen_t saddr_len = sizeof(saddr);

    printf("Connecting to server %s:%d from port %d\n", server_ip, server_port, client_port);

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("Client started. Type messages to send to server...\n");

    while (client_running) {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strlen(buffer) == 0) {
            continue;
        }

        char packet[BUFFER_SIZE];
        memset(packet, 0, BUFFER_SIZE);

        struct iphdr *iph = (struct iphdr *)packet;
        iph->version = 4;
        iph->ihl = 5;
        iph->tos = 0;
        iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(buffer) + 1);
        iph->id = htons(12345);
        iph->ttl = 64;
        iph->protocol = IPPROTO_UDP;
        iph->saddr = inet_addr("127.0.0.1");
        iph->daddr = inet_addr(server_ip);
        iph->check = 0;
        iph->check = csum((unsigned short *)iph, iph->ihl * 2);

        struct udphdr *udph = (struct udphdr *)(packet + sizeof(struct iphdr));
        udph->source = htons(client_port);
        udph->dest = htons(server_port);
        udph->len = htons(sizeof(struct udphdr) + strlen(buffer) + 1);
        udph->check = 0;

        char *data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
        memcpy(data, buffer, strlen(buffer) + 1);

        udph->check = udp_checksum(iph, udph, data, strlen(buffer) + 1);

        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = iph->daddr;
        dest_addr.sin_port = htons(server_port);
        
        printf("Sending to %s:%d\n", inet_ntoa(dest_addr.sin_addr), ntohs(dest_addr.sin_port));
        
        if (sendto(sockfd, packet, ntohs(iph->tot_len), 0, 
                   (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("sendto");
            continue;
        } else {
            printf("Message sent successfully\n");
        }

        memset(buffer, 0, BUFFER_SIZE);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&saddr, &saddr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            continue;
        }

        struct iphdr *reply_iph = (struct iphdr *)buffer;
        unsigned int iphdrlen = reply_iph->ihl * 4;
        struct udphdr *reply_udph = (struct udphdr *)(buffer + iphdrlen);

        if (reply_iph->protocol != IPPROTO_UDP || ntohs(reply_udph->dest) != client_port) {
            continue;
        }

        char *reply_data = (char *)(buffer + iphdrlen + sizeof(struct udphdr));

        printf("Server reply: %s\n", reply_data);
    }

    send_close_message(sockfd);
    close(sockfd);
    printf("Client stopped.\n");
    return 0;
}