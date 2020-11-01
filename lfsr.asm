.DATA
.ADDR x2000
.FILL #11

.CODE
.ADDR x0000

INIT
    CONST R6, #1               ; set counter to 1
    JSR LFSR_SUB               ; call subroutine
    JMP END                    ; jump to the end of the program

.FALIGN
LFSR_SUB
FOR_LOOP
    CONST R1, x00              ; setting register to data address
    HICONST R1, x20            ; setting register to data address
    
    LDR R0, R1, #0             ; load the number in R0
    
    SRL R1, R0, #15            ; shift bit 15 to the LSB
    AND R1, R1, #1             ; AND with 1
    SRL R2, R0, #13            ; shift bit 13 to the LSB
    AND R2, R2, #1             ; AND with 1
    SRL R3, R0, #12            ; shift bit 12 to the LSB
    AND R3, R3, #1             ; AND with 1
    SRL R4, R0, #10            ; shift bit 10 to the LSB
    AND R4, R4, #1             ; AND with 1
    
    XOR R5, R1, R2             ; XOR R2 and R3
    XOR R5, R5, R3             ; XOR R6 and R4
    XOR R5, R5, R4             ; XOR R6 and R5
    
    SLL R0, R0, #1             ; Shift R0 left by 1, get rid of MSB
    ADD R0, R0, R5             ; ADD the value of the XOR bit (0 or 1)
    
    CONST R1, x00              ; setting register to data address
    HICONST R1, x20            ; setting register to data address
    
    STR R0, R1, #0             ; store the new value in data mem
TEMP
    CMPI R0, #5                ; R0 - 5 (original value of seed is 5)
    BRz END_FOR_LOOP           ; If R0 - 5 = 0 go to END_FOR_LOOP
    ADD R6, R6, #1             ; R6 = R6 + 1 (increment counter)
    BRnzp FOR_LOOP             ; always go back to for loop    
END_LFSR_SUB
END_FOR_LOOP
RET
END                            ; end program




;=================================== OS ====================================;

;; A simple OS that just RTIs back to user code from the default PennSim entry point of x8200.

.OS
.CODE

.ADDR x80FF
HALT
	NOP

.ADDR x8200
.FALIGN
	CONST R7, #0
	RTI		; removes privilege bit
