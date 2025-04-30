#include "addition.h"
#include "division.h"
#include "multiplicate.h"
#include "substraction.h"
#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>
#define MAX 20

typedef double (*operations_func)(int, ...);
typedef struct {
    char name[MAX];
    char path[MAX];
    operations_func func;
}oper;

int main(){
    void *handle;
    int choice;
    int count;

    oper Oper[] = {
        {"addition", "./libaddition.so", NULL},
        {"substraction","./libsubstraction.so" ,NULL},
        {"multiplicate", "./libmultiplicate.so" ,NULL},
        {"division","./libdivision.so" ,NULL}
    };

    do
    {
        printf("\n-----Calculator Menu-----\n");

        printf("1. +\n");
        printf("2. -\n");
        printf("3. *\n");
        printf("4. /\n");
        printf("5. Exit\n");
        printf("Choose option.\n");
        scanf("%d", &choice);
        if(choice == 5) break;
        if(choice >=1 && choice <=4){
            handle = dlopen(Oper[choice - 1].path, RTLD_LAZY);
            if(!handle){
                fprintf(stderr, "Error loading library: %s\n", dlerror());
                return 1;
            }
            Oper[choice - 1].func = (operations_func)dlsym(handle, "calc");
            if(!Oper[choice - 1].func){
                fprintf(stderr, "Error loading function: %s\n", dlerror());
                dlclose(handle);
                return 1;
            }
        }
        printf("How many numbers you want to use: ");
        scanf("%d", &count);
        double numbers[count];
        for(int i = 0; i < count; i++){
            printf("Enter number: ");
            scanf("%lf", &numbers[i]);
        }
        double result = Oper[choice - 1].func(count, numbers[0], numbers[1], numbers[2], numbers[3], numbers[4], numbers[5], numbers[6]);
        printf("Result: %.2lf\n", result);
    } while (choice !=5);
    return 0;
}