#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

uint32_t IpToUint32(const char *ip) {
    uint32_t val = 0;
    uint8_t octet;
    char *ip_copy = strdup(ip);
    if (!ip_copy) return 0;
    
    char *token = strtok(ip_copy, ".");
    for (int i = 0; i < 4; ++i) {
        if (!token) {
            free(ip_copy);
            return 0;
        }
        octet = (uint8_t)atoi(token);
        val = (val << 8) | octet;
        token = strtok(NULL, ".");
    }
    
    free(ip_copy);
    return val;
}

int IsInSubnet(uint32_t ip, uint32_t subnet, uint32_t mask) {
    return (ip & mask) == (subnet & mask);
}

uint32_t GenerateRandomIP() {
    return ((uint32_t)rand() << 24) | 
           ((uint32_t)rand() << 16) | 
           ((uint32_t)rand() << 8) | 
           ((uint32_t)rand());
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <IP_address> <subnet_mask> <number_of_packets>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int N = atoi(argv[3]);
    if (N <= 0) {
        printf("Number of packets must be positive\n");
        return EXIT_FAILURE;
    }

    uint32_t gateway = IpToUint32(argv[1]);
    uint32_t mask = IpToUint32(argv[2]);

    if (gateway == 0 && strcmp(argv[1], "0.0.0.0") != 0) {
        printf("Invalid IP format: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (mask == 0 && strcmp(argv[2], "0.0.0.0") != 0) {
        printf("Invalid subnet mask format: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    int own_subnet_count = 0;
    for (int i = 0; i < N; ++i) {
        uint32_t random_ip = GenerateRandomIP();
        if (IsInSubnet(random_ip, gateway, mask)) {
            own_subnet_count++;
        }
    }

    printf("Packets in the same subnet: %d/%d (%.2f%%)\n", 
           own_subnet_count, N, 
           (double)own_subnet_count / N * 100);

    return EXIT_SUCCESS;
}