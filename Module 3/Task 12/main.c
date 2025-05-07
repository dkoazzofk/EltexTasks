#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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

    key_t key = ftok("shmfile", 65);

    int shmid = shmget(key, SHM_SIZE, 0666|IPC_CREAT);
    if(shmid == -1){
        perror("shmget");
        exit(1);
    }

    int *shm_ptr = (int*)shmat(shmid, NULL, 0);
    if(shm_ptr == (int*)(-1)){
        perror("shmat");
        exit(1);
    }

    if(pipe(pipefd) == -1){
        perror("pipe");
        return 1;
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
        shmdt(shm_ptr);
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
        shmdt(shm_ptr);
        shmctl(shmid, 0, NULL);
    }

    return 0;
}