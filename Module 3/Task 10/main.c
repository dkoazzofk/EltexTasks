#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_LENGTH_CHAR 128

int main(int argc, char *argv[]){
    int pipefd[2];
    pid_t pid;
    sem_t *sem;
    int value;

    sem = sem_open("semaphore", O_CREAT, 0644, 1);
    if(sem == NULL){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipefd) == -1){
        perror("pipe");
        sem_close(sem);
        sem_unlink("semaphore");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1){
        perror("fork");
        sem_close(sem);
        sem_unlink("semaphore");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        srand(time(NULL));
        close(pipefd[0]);
        while (1){
            sem_wait(sem);

            FILE *file = fopen("number.txt", "r");
            char name[MAX_LENGTH_CHAR];

            if (!file){
                perror("file\n");
                sem_post(sem);
                exit(EXIT_FAILURE);
            }
            while (fscanf(file, "%s", name) != EOF){
                printf("%s\n", name);
            }
            fclose(file);
            int random = rand() % 100;
            if (write(pipefd[1], &random, sizeof(random)) == -1){
                sem_post(sem);
                perror("write");
                exit(EXIT_FAILURE);
            }
            sem_post(sem);
            sleep(2);
        }
        close(pipefd[1]);
        return 0;
    } else {
        close(pipefd[1]);
        while (1){
            sem_wait(sem);
            int receivedNumber;

            if(read(pipefd[0], &receivedNumber, sizeof(receivedNumber)) == -1){
                perror("read\n");
                sem_post(sem);
                exit(EXIT_FAILURE);
            }

            FILE *file = fopen("number.txt", "a");
            if (!file){
                perror("file\n");
                sem_post(sem);
                exit(EXIT_FAILURE);
            }
            fprintf(file, "%d\n", receivedNumber);
            fclose(file);
            sem_post(sem);
            sleep(3);
        }
        wait(NULL);
        close(pipefd[0]);
        sem_close(sem);
        sem_unlink("semaphore");
    }
    return 0;
}