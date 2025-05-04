#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define MAX_LENGTH_CHAR 128
#define MAX_READERS 5

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


void initSemaphores(int semid) {
    union semun arg;
    
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl init write mutex");
        exit(EXIT_FAILURE);
    }
    
    arg.val = 0;
    if (semctl(semid, 1, SETVAL, arg) == -1) {
        perror("semctl init reader count");
        exit(EXIT_FAILURE);
    }
}

void startRead(int semid) {
    struct sembuf sops[2];
    
    sops[0].sem_num = 0;
    sops[0].sem_op = -1;
    sops[0].sem_flg = SEM_UNDO;
    
    sops[1].sem_num = 1;
    sops[1].sem_op = 1;
    sops[1].sem_flg = SEM_UNDO;
    
    if (semop(semid, sops, 2) == -1) {
        perror("semop start read");
        exit(EXIT_FAILURE);
    }
}

void endRead(int semid) {
    struct sembuf sops[2];
    
    sops[0].sem_num = 1;
    sops[0].sem_op = -1;
    sops[0].sem_flg = SEM_UNDO;
    
    sops[1].sem_num = 0;
    sops[1].sem_op = 1;
    sops[1].sem_flg = SEM_UNDO;
    
    if (semop(semid, sops, 2) == -1) {
        perror("semop end read");
        exit(EXIT_FAILURE);
    }
}

void startWrite(int semid) {
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = SEM_UNDO;
    
    if (semop(semid, &sops, 1) == -1) {
        perror("semop start write");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        int readers = semctl(semid, 1, GETVAL);
        if (readers == 0) break;
        usleep(10000);
    }
}

void endWrite(int semid) {
    struct sembuf sops;
    
    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = SEM_UNDO;
    
    if (semop(semid, &sops, 1) == -1) {
        perror("semop end write");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int pipefd[2];
    pid_t pid;

    key_t key = ftok(".", 's');
    int semid = semget(key, 2, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    initSemaphores(semid);

    if (pipe(pipefd)) {
        perror("pipe");
        return 1;
    }


    for (int i = 0; i < MAX_READERS; i++) {
        pid = fork();
        if (pid == 0) {
            srand(time(NULL) ^ getpid());
            close(pipefd[0]);
            
            while (1) {
                startRead(semid);
                
                FILE *file = fopen("number.txt", "r");
                char name[MAX_LENGTH_CHAR];

                if (!file) {
                    perror("file");
                    exit(EXIT_FAILURE);
                }
                
                printf("Reader %d read:\n", getpid());
                while (fscanf(file, "%s", name) != EOF) {
                    printf("%s ", name);
                }
                printf("\n");
                fclose(file);
                
                int random = rand() % 100;
                if (write(pipefd[1], &random, sizeof(random)) == -1) {
                    perror("write");
                }
                
                endRead(semid);
                sleep(1 + rand() % 3);
            }
            close(pipefd[1]);
            return 0;
        } else if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    close(pipefd[1]);
    while (1) {
        startWrite(semid);
        
        int receivedNumber;
        if (read(pipefd[0], &receivedNumber, sizeof(receivedNumber)) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        FILE *file = fopen("number.txt", "a");
        if (!file) {
            perror("file");
            exit(EXIT_FAILURE);
        }
        
        fprintf(file, "%d\n", receivedNumber);
        fclose(file);
        printf("Writer wrote: %d\n", receivedNumber);
        
        endWrite(semid);
        sleep(2);
    }

    while (wait(NULL) > 0);
    
    close(pipefd[0]);
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl remove");
    }
    
    return 0;
}