#include <stdio.h>
#include <stdarg.h>
#define MAX 20

double addition(int n, ...){
    double result = 0;
    va_list factor;
    va_start(factor, n);
    
    for(int i = 0; i < n; i++){
        result+= va_arg(factor, double);
    }
    va_end(factor);
    return result;
}

double substraction(int n, ...){
    double result;
    va_list factor;
    va_start(factor,n);
    result = va_arg(factor, double);

    for(int i = 1; i < n; i++){
        result-= va_arg(factor,double);
    }
    va_end(factor);
    return result;
}

double multiplicate(int n, ...){
    double result;
    va_list factor;
    va_start(factor,n);
    result = va_arg(factor, double);

    for(int i = 1; i < n; i++){
        result*= va_arg(factor,double);
    }
    va_end(factor);
    return result;
}

double division(int n, ...){
    double result;
    va_list factor;
    va_start(factor, n);
    result = va_arg(factor, double);

    for(int i = 1; i < n; i++){
        result /= va_arg(factor,double);
    }
    va_end(factor);
    return result;
}

typedef struct {
    char name[MAX];
    double (*function)(int n, ...);
}oper;

double operations(int choice, int count, double numbers[]){
    oper Oper[] = {
        {"addition", addition},
        {"substraction", substraction},
        {"multiplicate", multiplicate},
        {"division", division}
    };

    return Oper[choice - 1].function(count,numbers[0], numbers[1], numbers[2], numbers[3],
           numbers[4], numbers[5], numbers[6],numbers[7], numbers[8],numbers[9]);
}

int main(){
    int choice;
    int count;
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
        printf("How many numbers you want to use: ");
        scanf("%d", &count);
        double numbers[count];
        for(int i = 0; i < count; i++){
            printf("Enter number: ");
            scanf("%lf", &numbers[i]);
        }
        double result = operations(choice, count, numbers);
        printf("Result: %.2lf", result);
    } while (choice !=5);
    return 0;
}