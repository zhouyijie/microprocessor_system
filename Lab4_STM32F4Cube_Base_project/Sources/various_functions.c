
#include "various_functions.h"
#include "main.h"

//creates bi-geometric distribution for coeffecient values
 void generate_coeffecients(int order){
     int midpoint = order/2 + 1;
	   float sum = 0;
     int j;
     for(j=0; j < order; j++){
     coeff[j] = 1/(pow(2,(1+absolute(midpoint - j))));
     sum += coeff[j];
     }
     //ensure that the cumulative sum is equal to 1
     for(j=0; j< order; j++){
     coeff[j] = coeff[j]/sum;
     printf("%f, ",coeff[j]);
     }
     printf("\n");
 }
 
//Our FIR filter
int FIR_C(float* InputArray, float* OutputArray, float* coeff, int length, int order){
     int n;
     int z;
     float weighted_total;

     //apply the filter
     for(n = order; n< length; n++){
     //calculate n
     weighted_total = 0;
     //implement the weighting factor by the subset of the data array dicated by the order number
     for( z=0; z< order; z++){
     weighted_total += coeff[z] * InputArray[n- order + z];
     }
     OutputArray[n-order] = weighted_total;
     }
     return 0;
}

int absolute(int difference){
     if(difference <0)
     return (difference * -1);
     else
     return difference;
}