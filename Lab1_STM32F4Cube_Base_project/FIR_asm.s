 AREA text, CODE, READONLY
 EXPORT fir_asm

fir_asm
	LDM R3, {R3,R4}
	;VSTR.F32 S1, [R4]
	;VSTR.F32 S0, [R3]
loop1
	MOV R5, #1 ;instantiates counter at 1 per loop
	VSUB.F32 S6, S6, S6 ;set weight total 0
	MOV R6, R0 
	MOV R7, R2

loop2 ;loop to do each window
		
	VLDR.F32 S7, [R6]		;Loads input value from register
	VLDR.F32 S8, [R7]		;Loads coeff value from register	
	BVS stop ;prevents overflow	
	ADD R6,R6,#4			;4byte offset input
	ADD R7,R7,#4			;4byte offset bias
	VMLA.F32 S6, S7, S8
	ADD R5, R5, #1		;increments order by 1 value	
	CMP R5,R4			;is array 0
	BLE	loop2			;branch to loop2
;end of loop 2	
	VSTR.F32 S6, [R1]		;store value to output array	
	ADD	R1,R1, #4	;increment by 4bytes
	ADD R0,R0, #4
	SUB R3,R3,#1		;decrements array by 1 value
	CMP R3,#0			;is array 0
	BNE	loop1			;branch to loop
stop
	BX LR
		
	END