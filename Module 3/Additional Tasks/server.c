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
#define MAX_CLIENTS 100
#define DEFAULT_PORT 9090

typedef struct {
    struct in_addr ip;
    unsigned short port;
    int count;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int server_running = 1;
unsigned short server_port = DEFAULT_PORT;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        server_running = 0;
    }
}

int find_client_index(struct in_addr ip, unsigned short port) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].ip.s_addr == ip.s_addr && clients[i].port == port) {
            return i;
        }
    }
    return -1;
}

int add_client(struct in_addr ip, unsigned short port) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].ip.s_addr == 0 && clients[i].port == 0) {
            clients[i].ip = ip;
            clients[i].port = port;
            clients[i].count = 0;
            return i;
        }
    }
    return -1;
}

void remove_client(struct in_addr ip, unsigned short port) {
    int index = find_client_index(ip, port);
    if (index != -1) {
        memset(&clients[index], 0, sizeof(ClientInfo));
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

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in saddr;
    socklen_t saddr_len = sizeof(saddr);

    printf("Starting server on port %d\n", server_port);

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

    printf("Server started. Waiting for messages...\n");

    while (server_running) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&saddr, &saddr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            continue;
        }

        struct iphdr *iph = (struct iphdr *)buffer;
        unsigned int iphdrlen = iph->ihl * 4;

        struct udphdr *udph = (struct udphdr *)(buffer + iphdrlen);

        if (iph->protocol != IPPROTO_UDP || ntohs(udph->dest) != server_port) {
            continue;
        }

        char *data = (char *)(buffer + iphdrlen + sizeof(struct udphdr));
        int data_len = ntohs(udph->len) - sizeof(struct udphdr);

        if (strcmp(data, "CLOSE_CONNECTION") == 0) {
            printf("Client %s:%d closed connection\n", inet_ntoa(*(struct in_addr *)&iph->saddr), ntohs(udph->source));
            remove_client(*(struct in_addr *)&iph->saddr, ntohs(udph->source));
            continue;
        }

        int client_index = find_client_index(*(struct in_addr *)&iph->saddr, ntohs(udph->source));
        if (client_index == -1) {
            client_index = add_client(*(struct in_addr *)&iph->saddr, ntohs(udph->source));
            if (client_index == -1) {
                printf("Too many clients\n");
                continue;
            }
        }

        clients[client_index].count++;
        
        char reply[BUFFER_SIZE];
        snprintf(reply, BUFFER_SIZE, "%s %d", data, clients[client_index].count);

        printf("Received from %s:%d: %s\n", inet_ntoa(*(struct in_addr *)&iph->saddr), ntohs(udph->source), data);
        printf("Sending reply: %s\n", reply);

        char packet[BUFFER_SIZE];
        memset(packet, 0, BUFFER_SIZE);

        struct iphdr *reply_iph = (struct iphdr *)packet;
        reply_iph->version = 4;
        reply_iph->ihl = 5;
        reply_iph->tos = 0;
        reply_iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(reply) + 1);
        reply_iph->id = htons(54321);
        reply_iph->ttl = 64;
        reply_iph->protocol = IPPROTO_UDP;
        reply_iph->saddr = iph->daddr;
        reply_iph->daddr = iph->saddr;
        reply_iph->check = 0;
        reply_iph->check = csum((unsigned short *)reply_iph, reply_iph->ihl * 2);

        struct udphdr *reply_udph = (struct udphdr *)(packet + sizeof(struct iphdr));
        reply_udph->source = udph->dest;
        reply_udph->dest = udph->source;
        reply_udph->len = htons(sizeof(struct udphdr) + strlen(reply) + 1);
        reply_udph->check = 0;

        char *reply_data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
        memcpy(reply_data, reply, strlen(reply) + 1);

        reply_udph->check = udp_checksum(reply_iph, reply_udph, reply_data, strlen(reply) + 1);

        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = reply_iph->daddr;
        dest_addr.sin_port = udph->source;
        
        if (sendto(sockfd, packet, ntohs(reply_iph->tot_len), 0, 
                   (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("sendto");
        } else {
            printf("Sent reply to %s:%d\n", 
                   inet_ntoa(dest_addr.sin_addr), ntohs(dest_addr.sin_port));
        }
    }

    close(sockfd);
    printf("Server stopped.\n");
    return 0;
}