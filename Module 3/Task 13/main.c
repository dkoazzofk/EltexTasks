#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define SHM_SIZE 1024

sig_atomic_t stopFlag = 0;

void sigintHandler(int sig) {
    stopFlag = 1;
}

void generateNumbers(int *shm_ptr, int count){
    shm_ptr[0] = count;
    for(int i = 1; i < count; i++){
        shm_ptr[i] = rand() % 20;
    }
}

void findMinMax(int *shm_ptr){
    int count = shm_ptr[0];
    
    int min = shm_ptr[1];
    int max = shm_ptr[1];

    for(int i = 2; i <= count; i++){
        if(shm_ptr[i] < min) min = shm_ptr[i];
        if(shm_ptr[i] > max) max = shm_ptr[i];
    }
    shm_ptr[count + 1] = min;
    shm_ptr[count + 2] = max;
}

int main(){
    signal(SIGINT, sigintHandler);
    srand(time(NULL));
    int number;
    int pipefd[2];
    pid_t pid;
    int processedCount = 0;

    int shm_fd = shm_open("/shm_fd", O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){
        perror("shm_open");
        exit(1);
    }

    if(ftruncate(shm_fd, 1024) == -1){
        perror("ftruncate");
        exit(1);
    }

    int *shm_ptr = (int*)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm_ptr == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    pid = fork();

    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    } else if(pid == 0){
        printf("Child process\n");
        while(!stopFlag){
            findMinMax(shm_ptr);
            sleep(1);
        }
        munmap(shm_ptr, 1024);
        close(shm_fd);
        exit(0);
    } else{
        printf("Parent process\n");
        while(!stopFlag){
            int count = 2 + rand() % 5;
            generateNumbers(shm_ptr, count);
            printf("Generated numbers(Count: %d): ", count);
            for(int i = 1; i <= count; i++){
                printf("%d ", shm_ptr[i]);
            }
            printf("\n");
            sleep(1);
            printf("Min: %d\nMax: %d", shm_ptr[count + 1], shm_ptr[count + 2]);
            processedCount++;
        }
        printf("Data processed: %d", processedCount);
        munmap(shm_ptr, 1024);
        close(shm_fd);
        shm_unlink("/shm_fd");
    }

    return 0;
}