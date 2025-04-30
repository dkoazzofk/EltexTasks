#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

int calculate(int lenght){
    return lenght*lenght;
}

int main(int argc, char *argv[]){

    int *side_lenght = NULL;

    if(!(side_lenght = (int*) malloc(argc-1 * sizeof(int)))){
        printf("Ошибка выделения памяти");
        return -1;
    }

    for(int i = 0; i < argc - 1; i++){
        side_lenght[i] = atoi(argv[i+1]);
    }

    int half = (argc-1)/2;

    pid_t pid;

    switch (pid = fork())
    {
    case -1:
        perror("Error.\n");
        free(side_lenght);
    case 0:
        printf("Child process.\n");
        for(int i = 0; i < half; i++){
            printf("Square area %d: %d\n",i+1, calculate(side_lenght[i]));
        }
        break;
    default:
        printf("Parent process.\n");
        for(int i = half; i < argc - 1; i++){
            printf("Square area %d: %d\n",i, calculate(side_lenght[i]));
        }
        break;
    }
    free(side_lenght);
    return 0;
}