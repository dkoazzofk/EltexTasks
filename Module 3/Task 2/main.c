#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_LENGTH_STRING 126
#define MAX_ARGS 10


int main() {
    char input[MAX_LENGTH_STRING];
    char *args[MAX_ARGS+1];
    for (;;){
        printf(">> ");

        if (fgets(input, MAX_LENGTH_STRING, stdin) != NULL) {
            if (input[strlen(input) - 1] == '\n') {
                input[strlen(input) - 1] = '\0';
            }
        }
        char *arg = strtok(input," ");
        int argCount = 0;
        while (arg != NULL && argCount < 11) {
            args[argCount++] = arg;
            arg = strtok(NULL, " ");
        }
        args[argCount] = NULL;

        if (argCount == 0){
            continue;
        }

        if (strcmp(args[0], "exit") == 0){
            printf("Exiting...\n");
            break;
        }

        pid_t pid;
    
        switch (pid = fork()) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            execv(args[0],args);
            puts("FIle doesn't exist.\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}