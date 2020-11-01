

.CODE
MAIN
    CONST R0, #-1              ; A = 6
    JSR SUB_FACTORIAL          ; jump to subroutine - SUB_FACTORIAL
    JMP END                    ; jump to end
    
.FALIGN   
SUB_FACTORIAL
    CMPI R0, #0                ; sets NZP (A - 0)
    BRn EDGE_CASE_LABEL
    CMPI R0, #7                ; sets NZP (A - 7)
    BRp EDGE_CASE_LABEL
    ADD R1, R0, #0             ; B = A
    
LOOP
    CMPI R0, #1                ; sets NZP (A - 1)
    BRnz END_SUB_FACTORIAL     ; test NZP (was zero or negative - END_SUB_FACTORIAL)    
    ADD R0, R0, #-1            ; A = A - 1
    MUL R1, R1, R0             ; B = B * A
    BRnzp LOOP                 ; always go to loop 
END_SUB_FACTORIAL              ; end subroutine
    RET                        ; return program counter
    
EDGE_CASE_LABEL
    CONST R1, #-1
    RET
END                            ; end program