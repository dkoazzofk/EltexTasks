#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LENGTH_CHAR 128

volatile sig_atomic_t access_flag = 0; 

void sigusr1_handler(int signum) {
    access_flag = 1; 
}

void sigusr2_handler(int signum) {
    access_flag = 0; 
}

int main(int argc, char *argv[]){
    int pipefd[2];
    pid_t pid;

    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
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
            while(!access_flag){
                pause();
            }
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
        while (1){
            kill(pid, SIGUSR1);
            
            int receivedNumber;

            FILE *file = fopen("number.txt", "a");
            if (!file){
                perror("file\n");
                exit(EXIT_FAILURE);
            }

            if(read(pipefd[0], &receivedNumber, sizeof(receivedNumber)) == -1){
                perror("read\n");
                exit(EXIT_FAILURE);
            }

            fprintf(file, "%d\n", receivedNumber);
            fclose(file);
            kill(pid, SIGUSR2);
            sleep(3);
            close(pipefd[1]);
        }
        wait(NULL);
        close(pipefd[0]);
    }

    return 0;
}