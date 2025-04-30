#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define MAX_LENGTH_CHAR 128

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
}

void semWait(int semid){
    struct sembuf sb = {0, -1, 0};
    if(semop(semid, &sb, 1) == -1){
        perror("semop waint");
        exit(EXIT_FAILURE);
    }
}

void semSignal(int semid){
    struct sembuf sb = {0, 1, 0};
    if(semop(semid, &sb, 1) == -1){
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char *argv[]){
    int pipefd[2];
    pid_t pid;

    key_t key = ftok(".", 's');
    int semid = semget(key, 1, 0666 | IPC_CREAT);
    if(semid == -1){
        perror("semget");
        exit(EXIT_FAILURE);
    }

    union semun arg;
    arg.val = 1;
    if(semctl(semid, 0, SETVAL, arg) == -1){
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    if(pipe(pipefd) == -1){
        perror("pipe");
        return 1;
    }

    pid = fork();
    if (pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        srand(time(NULL));
        close(pipefd[0]);
        while (1){
            semWait(semid);

            FILE *file = fopen("number.txt", "r");
            char name[MAX_LENGTH_CHAR];

            if (!file){
                perror("file\n");
                exit(EXIT_FAILURE);
            }
            while (fscanf(file, "%s", name) != EOF){
                printf("%s\n", name);
            }
            fclose(file);
            int random = rand() % 100;
            if (write(pipefd[1], &random, sizeof(random)) == -1){
                perror("write");
            }
            sleep(2);
        }
        close(pipefd[1]);
        return 0;
    } else {
        close(pipefd[1]);
        while (1){
            semWait(semid);
            int receivedNumber;

            if(read(pipefd[0], &receivedNumber, sizeof(receivedNumber)) == -1){
                perror("read\n");
                exit(EXIT_FAILURE);
            }

            FILE *file = fopen("number.txt", "a");
            if (!file){
                perror("file\n");
                exit(EXIT_FAILURE);
            }
            fprintf(file, "%d\n", receivedNumber);
            fclose(file);
            semSignal(semid);
            sleep(3);
        }
        wait(NULL);
        close(pipefd[0]);
        if(semctl(semid, 0, IPC_RMID) == -1){
            perror("semctl remove");
        }
    }
    return 0;
}