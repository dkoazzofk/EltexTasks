#include "division.h"

double calc(int n, ...){
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