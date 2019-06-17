AREA codespace, CODE, READONLY

IMPORT input_data_array
IMPORT filter_coeffecients
IMPORT array_length
	
EXPORT filtered_data
	
filtered_data
	MOV 
	LDR R0, =input_data_array	
	LDR S0, =filter_coeffecients	
	LDR R1, =array_length	
	VMUL S1, R0, S0 
	

	
END