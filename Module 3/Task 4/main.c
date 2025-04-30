#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

int main(){
    int number;
    int pipefd[2];
    pid_t pid;

    if(pipe(pipefd) == -1){
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        number = rand() % 20;
        close(pipefd[0]);
        write(pipefd[1], &number, sizeof(int));
        close(pipefd[1]);
    } else {
        printf("Parent process.\n");
        read(pipefd[0], &number, sizeof(int));
        printf("Number = %d", number);

        FILE *file = fopen("number.txt", "w");
        if (file == NULL){
            perror("file");
            exit(EXIT_FAILURE);
        }
        fprintf(file, "%d", number);
        close(pipefd[1]);
        close(pipefd[0]);
        fclose(file);
        exit(EXIT_SUCCESS);
    }

    return 0;
}