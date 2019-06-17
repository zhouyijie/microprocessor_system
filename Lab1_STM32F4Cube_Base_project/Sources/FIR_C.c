#include <stdio.h>
//#include "arm_math.h"



int filter(float* InputArray, float* OutputArray, float*
coeff, int Length, int Order)
{
	float *InputArrayp;
	float *coeffp;
	int n;
	int z;
	float weighted_total;

	//apply the filter
	for(n = 4; n< Length; n++){
		//calculate n
		coeffp = coeff;
		weighted_total = 0;
		InputArrayp = InputArray;
		//implement the weighting factor by the subset of the data array dicated by the order number
		for( z=0; z< Order; z++){
			weighted_total += (*coeffp++) * (*InputArrayp--);
		}
		*(OutputArray + n) = weighted_total;
	}
	
	return 0;
}

