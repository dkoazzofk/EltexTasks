#include <stdio.h>
#include <stdlib.h>
#define MAX_LENGHT 10


int main(int argc, char *argv[]){
    int max = atoi(argv[0]);
    for (int i = 0; i < argc; i++){
        for (int i = 2; i < argc; i++){
            int num = atoi(argv[i]);
            if (num > max){
                max = num;
            }
        }
    }
    printf("Max: %d\n", max);
    return 0;
}