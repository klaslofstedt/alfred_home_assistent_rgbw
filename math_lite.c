#include "math_lite.h"

float power(float base, float exponent)
{
    float temp = 1;
    if (base <= 0){
        return -1;
    }
    if (exponent <= 0){
        return 1;
    }
    uint8_t i;
    for (i = 1; i <= exponent; i++){
        temp *= base;
    }
    return(temp);
}

