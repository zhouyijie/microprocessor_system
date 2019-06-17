#include <stdio.h>
#include "arm_math.h"

extern int fir_asm (float* InputArray, float* OutputArray, float* coeff, int* param);
int FIR_C(float* InputArray, float* OutputArray, float* coeff, int Length, int Order);

//part 2
void subtraction(float* sub, float* in1, float* in2, int length2);
//used within std_dev and correlation
float mean(float* input, int length);
float std_dev(float* data, int length);
void correlation(float* in1, float* in2, float* out, int length);
//in1 is the original, in2 the filtered 
float average_difference(float* in1, float *in2, int length2);


//Code for the C filter itself
int FIR_C(float* InputArray, float* OutputArray, float* coeff, int Length, int Order)
{
	int n;
	int z;
	float weighted_total;

	//apply the filter
	for(n = 0; n< Length; n++){
		//calculate n
		weighted_total = 0;
		//implement the weighting factor by the subset of the data array dicated by the order number
		for( z=0; z< Order; z++){
			weighted_total += coeff[z] * InputArray[n + z];
		}
		*(OutputArray++) = weighted_total;
	}
	
	return 0;
}


//Part 2 helper functions

float average_difference(float* in1, float *in2, int length){
	int i;
	int avg = 0;
	for(i =0; i<length; i++){
		avg += (in1[i] - in2[i]);
	}
	avg = avg/length;
	return avg;
}

//subtraction begins with the input Array's fourth value
void subtraction(float* sub, float* in1, float* in2, int length2){
	int i;
	for(i = 0; i < length2; i++){
		sub[i] = in1[i] - in2[i];
	}
}

float mean(float* input, int length){
    
	int i;
	float sum = 0;
	for(i = 0; i < length; i++){
		sum += input[i];
	}
	return (sum / length);
}

float std_dev(float* data, int length){
	float standard_deviation = 0.0;
	int i;
	float dev_mean = mean(data, length);
	
	for(i=0; i<length; i++)
        standard_deviation += powf(data[i] - dev_mean, 2.0);
	
	return sqrtf(standard_deviation/length);
}

void correlation(float* in1, float* in2, float* out, int length){
	int i;
	int j;
	int k_min, k_max;
	for(i = 0; i < (2*length-1); i++){
		out[i] = 0;
	//	k_min = (i < length) ? 0 : i - length+1;
		k_min = 0;
		k_max = (i < length - 1) ? i : length - 1;
		for(j=k_min; j<k_max; j++){
			out[i] += in1[j]*in2[length-j-i+1];
		}
	}
}




//Main starts here----------------------------------------------
int main()
{
	float InputArray[20] = {1.0, 1.07, 1.15, 1.2, 1.25, 1.3, 1.358, 1.39, 1.15, 1.2, 1.15, 1.1, 1.05, 1.0, 0.8, 0.6, 0.4, 0.0, -0.3, -0.8};
	float coeff[5] = {.1f,.15f,.5f,.15f,.1f};
	const int length = sizeof(InputArray)/4; //Need the 4 because the length is measured per bit, and these are bytes
	const int Order = 5;
	const int length_out = length;
	// const error: convert integer to constant for length order and length_out
	float OutputArray[length];
	float OutputArray_CMSIS[length];
	int i;
	int param[] = {length, Order};
	float outputasm[length];
	float Cdiff[length_out];
	float CMSISdiff[length_out];
	float stdC;
	float stdCMSIS;
	float corr_c[length*2 - 1];
	float correlation_CMSIS[2*length-1];
	printf("Length of input Array is %i\n", length);

	printf("Begin C code:\n");

	//C code
	FIR_C(InputArray, OutputArray, coeff, length, Order);

	for(i=0; i<(length); i++)
	{
		printf("%.3f, ",OutputArray[i]);
	}
	
	printf("\nBegin Assembly:\n");

	//assembly
	fir_asm(InputArray, outputasm, coeff, param);
	for(i=0; i<(length); i++)
	{
		printf("%.3f, ",outputasm[i]);
	}
	
	printf("\nBegin CMSIS:\n");
	float32_t firStateF32[length + 5 - 1];
	arm_fir_instance_f32 S;
	arm_fir_init_f32(&S, 5, coeff, &firStateF32[0], 5);
	for(i=0; i<length; i++)
		arm_fir_f32( &S, InputArray	, OutputArray_CMSIS, length);
		for(i=0; i<(length); i++)
	{
		printf("%.3f, ",OutputArray_CMSIS[i]);
	}
	//---------- BEGIN CALLING HELPER FUNCTIONS---------------
	
	//subtraction
	printf("\nBegin Subtraction:\n");
	subtraction(Cdiff, InputArray, OutputArray, length_out);
	arm_sub_f32(InputArray, OutputArray, CMSISdiff, length_out);
	printf("C code output:\n");
	for(i=0; i<(length); i++)
	{
		printf("%.3f, ",Cdiff[i]);
	}
	printf("\nCMSIS code output:\n");
	for(i=0; i<(length); i++)
	{
		printf("%.3f, ",CMSISdiff[i]);
	}
	
	//std_deviation
	printf("\nBegin Sd Deviation:\n");
	arm_std_f32(CMSISdiff, length_out, &stdCMSIS);
	stdC = std_dev(Cdiff, length_out);
	printf("C code output:%.2f",stdC);
	printf("\nCMSIS code output:%.2f",stdCMSIS);
	
	//correlation
	printf("\nBegin Correlation:\n");
	correlation(InputArray, OutputArray, corr_c, length_out);
	arm_correlate_f32 (InputArray, length, OutputArray, length, correlation_CMSIS);
	printf("C code output: ");	
	for(i=0; i<(2*length-1); i++)
	{
		printf("%.3f, ",corr_c[i]);
	}
	printf("\nCMSIS code output:\n");
	for(i=0; i<(2*length-1); i++)
	{
		printf("%.3f, ",correlation_CMSIS[i]);
	}
	
	return 0;

}
