#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_LENGTH_CHAR 128
#define MAX_READERS 5

int main(int argc, char *argv[]) {
    int pipefd[2];
    pid_t pid;
    sem_t *readers_mutex, *resource_mutex;
    int readers_count = 0;

    readers_mutex = sem_open("readers_mutex", O_CREAT, 0644, 1);
    resource_mutex = sem_open("resource_mutex", O_CREAT, 0644, 1);
    
    if (readers_mutex == SEM_FAILED || resource_mutex == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        sem_close(readers_mutex);
        sem_close(resource_mutex);
        sem_unlink("readers_mutex");
        sem_unlink("resource_mutex");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        sem_close(readers_mutex);
        sem_close(resource_mutex);
        sem_unlink("readers_mutex");
        sem_unlink("resource_mutex");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        srand(time(NULL));
        close(pipefd[0]);
        
        while (1) {
            sem_wait(readers_mutex);
            readers_count++;
            if (readers_count == 1) {
                sem_wait(resource_mutex);
            }
            sem_post(readers_mutex);

            FILE *file = fopen("number.txt", "r");
            char name[MAX_LENGTH_CHAR];

            if (!file) {
                perror("file");
                sem_wait(readers_mutex);
                readers_count--;
                if (readers_count == 0) {
                    sem_post(resource_mutex);
                }
                sem_post(readers_mutex);
                exit(EXIT_FAILURE);
            }
            
            while (fscanf(file, "%s", name) != EOF) {
                printf("Reader: %s\n", name);
            }
            fclose(file);

            int random = rand() % 100;
            if (write(pipefd[1], &random, sizeof(random)) == -1) {
                perror("write");
            }

            sem_wait(readers_mutex);
            readers_count--;
            if (readers_count == 0) {
                sem_post(resource_mutex);
            }
            sem_post(readers_mutex);

            sleep(2);
        }
        close(pipefd[1]);
        return 0;
    } else {
        close(pipefd[1]);
        
        while (1) {
            int receivedNumber;
            if (read(pipefd[0], &receivedNumber, sizeof(receivedNumber)) == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            sem_wait(resource_mutex);

            FILE *file = fopen("number.txt", "a");
            if (!file) {
                perror("file");
                sem_post(resource_mutex);
                exit(EXIT_FAILURE);
            }
            fprintf(file, "%d\n", receivedNumber);
            fclose(file);
            printf("Writer: wrote %d\n", receivedNumber);
            
            sem_post(resource_mutex);
            sleep(3);
        }
        
        wait(NULL);
        close(pipefd[0]);
        sem_close(readers_mutex);
        sem_close(resource_mutex);
        sem_unlink("readers_mutex");
        sem_unlink("resource_mutex");
    }
    return 0;
}