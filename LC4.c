/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

#define INSN_OP(I) ((I) >> 12)
#define INSN_SUB_OP(I) (((I) >> 3) & 0x7)
#define INSN_11_9(I) (((I) >> 9) & 0x7) 
#define INSN_8_6(I) (((I) >> 6) & 0x7)
#define INSN_2_0(I) ((I) & 0x7)
#define INSN_4_0(I) ((I) & 0x1F)
#define INSN_OP_5(I) ((I) >> 11)
#define INSN_15_9(I) ((I) >> 9)
#define INSN_5_4(I) (((I) >> 4) & 0x3)
#define INSN_5_0(I) ((I) & 0x3F)
#define INSN_8_7(I) (((I) >> 7) & 0x3)

unsigned short int storePC;
unsigned short int whichRegister;

/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState* CPU) {
	CPU->PC = 0x8200;

	CPU->PSR = 0x8002;
    
    memset(CPU->memory, 0, 65536 * 2);
    
    ClearSignals(CPU);

}

/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState* CPU) {
    
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    
    CPU->dmemValue = 0;
    CPU->dmemAddr = 0;

}
/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState* CPU, FILE* output) {
	
	int decToBin[16];
	int i;
	unsigned short int inst;
	
    for (i = 0; i < 16; i++) {
        decToBin[i] = 0;
    }
	// print PC 
	fprintf(output, "%04X ", storePC);
    //printf("%d\n", storePC);

	// convert decimal to binary
	inst = CPU->memory[storePC];
    
    //printf("%d\n", inst);
    //printf("%x\n", inst);
    
	i = 0;
    
	while (inst > 0) {
		decToBin[i] = inst % 2;
		inst = inst / 2;
		i++;
	}

	for (i = 15; i >= 0; i--) {
        if (decToBin[i] == 0) {
            fprintf(output, "%d", 0);   
        } else if (decToBin[i] == 1) {
            fprintf(output, "%d", 1);
        }	
	}
	fputs(" ", output);
	
	//print register_WE
	fprintf(output, "%d ", CPU->regFile_WE);
	
	if (CPU->regFile_WE == 1) {
		fprintf(output, "%d ", whichRegister);
        fprintf(output, "%04X ", CPU->regInputVal);
	} else if (CPU->regFile_WE == 0) {
		fprintf(output, "%d ", 0);
        fprintf(output, "%04X ", 0);
	}
    
    //print NZP_WE
    fprintf(output, "%d ", CPU->NZP_WE);
    
    if (CPU->NZP_WE == 1) {
        fprintf(output, "%d ", CPU->NZPVal);
    } else if (CPU->NZP_WE == 0){
		fprintf(output, "%d ", 0);
	}
	
	//print DATA_WE, dmemAddr, dmemValue (default 0)
	fprintf(output, "%d ", CPU->DATA_WE);
    fprintf(output, "%04X ", CPU->dmemAddr);
	fprintf(output, "%04X ", CPU->dmemValue);

    //print line
    fputs("\n", output);
}

/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState* CPU, FILE* output) {
    int i;
    unsigned short int inst;
    unsigned short int oppcode;
    unsigned short int tempPSR;
    unsigned short int destReg;
    unsigned short int srcReg;
    unsigned short int rtReg;
    unsigned short int x;
    unsigned short int v;
    unsigned short int dummyPSR;
    unsigned short int result;
    int y;
    
    if (CPU->PC == 0x80FF) {
        printf("Entering the trap table, time to leave\n");
        return 0;
    }
    
    if ((CPU->PC >= 0x2000 && CPU->PC <= 0x7FFF) ||
       (CPU->PC >= 0xA000 && CPU->PC <= 0xFFFF)) {
        printf("Attempting to execute data section as code\n");
        return 1;
    }
    
    dummyPSR = CPU->PSR;
    y = dummyPSR >> 15;
    
    if (CPU->PC >= 0x8000 && y == 0) {
        printf("Privilege mode is off");
        return 2;
    }
    
    inst = CPU->memory[CPU->PC];
	storePC = CPU->PC;
    oppcode = INSN_OP(inst);

    switch(oppcode) {
        case 0 : // 0000 - Branch
            ClearSignals(CPU);
            BranchOp(CPU, output);
            break;
            
        case 1 : // 0001 - Arithmetic
            ClearSignals(CPU);
            ArithmeticOp(CPU, output);
            break;
            
        case 2 : // 0010 - Compare 
            ClearSignals(CPU);
            ComparativeOp(CPU, output);
            break;
            
        case 4 : // 0100 - (0/1) - JSR 
            
            ClearSignals(CPU);
            JSROp(CPU, output);
            break;
            
        case 5 : // 0101 - logical
            ClearSignals(CPU);
            LogicalOp(CPU, output);
            break;
            
        case 6 : // 0110 - LDR
            
            ClearSignals(CPU);
            inst = CPU->memory[CPU->PC];
            destReg = INSN_11_9(inst);
            srcReg = INSN_8_6(inst);
            
            x = INSN_5_0(inst);
            y = x >> 5;
            if (y == 1) {
                x = x + 0xFFC0;
            }
            
            if (CPU->R[srcReg] + x <= 0x1FFF 
                && CPU->R[srcReg] + x >= 0x0000) {
                printf("invalid load/store address\n");
                return 3;
            }
            
            if (CPU->R[srcReg] + x >= 0x8000 &&
               CPU->R[srcReg] + x <= 0x9FFF) {
                printf("invalid load/store address\n");
                return 3;
            }
            
            dummyPSR = CPU->PSR;
            y = dummyPSR >> 15;
            
            if ((CPU->R[srcReg] + x >= 0xA000 &&
               CPU->R[srcReg] + x <= 0xFFFF) && y == 0 ) {
                printf("invalid load/store address in OSdata\n");
                return 4;
            } 
            
            // whichRegister
            whichRegister = destReg;
            
            CPU->dmemAddr = CPU->R[srcReg] + x;
            CPU->dmemValue = CPU->memory[CPU->dmemAddr];
            CPU->regFile_WE = 1;
            
            // actually puts value in register
            CPU->R[destReg] = CPU->dmemValue;
            // regrade changes made here
            CPU->regInputVal = CPU->dmemValue;
            
            result = CPU->R[destReg];
            x = result >> 15;
    
            CPU->NZP_WE = 1;
            if (x == 1) {
                SetNZP(CPU, 4);
            } else if (result == 0) {
                SetNZP(CPU, 2);
            } else if (result > 0) {
                SetNZP(CPU, 1);
            }       
            
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;

        case 7 : // 0111 - STR 
            
            ClearSignals(CPU);
            inst = CPU->memory[CPU->PC];
            rtReg = INSN_11_9(inst);
            srcReg = INSN_8_6(inst);
            
            x = INSN_5_0(inst);
            y = x >> 5;
            
            if (y == 1) {
                x = x + 0xFFC0;
            }
            
            if (CPU->R[srcReg] + x <= 0x1FFF 
                && CPU->R[srcReg] + x >= 0x0000) {
                printf("invalid load/store address\n");
                return 3;
            }
            
            if (CPU->R[srcReg] + x >= 0x8000 && 
               CPU->R[srcReg] + x <= 0x9FFF) {
                printf("invalid load/store address\n");
                return 3;
            }
            
            dummyPSR = CPU->PSR;
            y = dummyPSR >> 15;
            
            if ((CPU->R[srcReg] + x >= 0xA000 &&
               CPU->R[srcReg] + x <= 0xFFFF) && y == 0 ) {
                printf("invalid load/store address in OSdata\n");
                return 4;
            }
            
            //regrade changes made here
            CPU->DATA_WE = 1;
            CPU->dmemAddr = CPU->R[srcReg] + x;
            CPU->dmemValue = CPU->R[rtReg];
            
            CPU->memory[CPU->dmemAddr] = CPU->dmemValue;
           
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;

        case 9 : // 1001 Const
            ClearSignals(CPU);
            inst = CPU->memory[CPU->PC];
            destReg = INSN_11_9(inst);
            x = inst;
            x = x & 0x01FF;
            y = x >> 8;
            if (y == 1) {
                // changed this from 0xFC00 to 0xFE00
                // regrade changes
                x = x + 0xFE00;
                //SetNZP(CPU, 4);
            }
            
            CPU->R[destReg] = x;
            CPU->regInputVal = x;
            v = x >> 15;
            CPU->NZP_WE = 1;
            CPU->DATA_WE = 0;
            CPU->regFile_WE = 1;
            whichRegister = destReg;
            
            // call to setNZP
            // regrade change made here
            if (v == 1) {
                SetNZP(CPU, 4);
            } else if (x == 0) {
                SetNZP(CPU, 2);
            } else if (x > 0) {
                SetNZP(CPU, 1);
            }
            
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
            
        case 8 : // RTI 
            
            ClearSignals(CPU);
            CPU->PC = CPU->R[7];
            
            tempPSR = CPU->PSR; 
            tempPSR = tempPSR & 0x7;
            CPU->PSR = tempPSR;
            CPU->NZP_WE = 0;
            CPU->DATA_WE = 0;
            CPU->regFile_WE = 0; 
    
            WriteOut(CPU, output);
            break;
            
        case 10 : // 1010 Shift
            ClearSignals(CPU);
            ShiftModOp(CPU, output);
            
            break;
        case 12 : // 1100 Jump stuff
            ClearSignals(CPU);
            JumpOp(CPU, output);
            break;
            
        case 13 : // Hiconst
            ClearSignals(CPU);
            inst = CPU->memory[CPU->PC];
            
            destReg = INSN_11_9(inst);
            x = inst & 0xFF;
            v = CPU->R[destReg];
            
            CPU->R[destReg] = ((v & 0xFF ) | (x << 8));
            
            whichRegister = destReg;
            CPU->regInputVal = CPU->R[destReg];
            CPU->regFile_WE = 1;
            CPU->NZP_WE = 1;

            result = CPU->R[destReg];
            // set NZP
            v = result >> 15;
    
            if (v == 1) {
                SetNZP(CPU, 4);   
            } else if (result == 0) {
                SetNZP(CPU, 2);
            } else if (result > 0) {
                SetNZP(CPU, 1);
            }
            
            WriteOut(CPU, output);
            CPU->PC = CPU->PC + 1;
            break;
           
        case 15 : // Trap
            ClearSignals(CPU);
            inst = CPU->memory[CPU->PC];
            
            CPU->R[7] = CPU->PC + 1;
            //dummyPC = CPU->PC;
            x = inst & 0xFF;
            CPU->PC = (0x8000 | x);
            
            CPU->NZP_WE = 1;
            CPU->regFile_WE = 1;
            whichRegister = 7;
            CPU->regInputVal = CPU->R[7];
            
            dummyPSR = CPU->PSR;
            dummyPSR = dummyPSR & 0x7;
            CPU->PSR = 0x8000 + dummyPSR;
            
            result = CPU->R[7];
            // set NZP
            v = result >> 15;
    
            if (v == 1) {
                SetNZP(CPU, 4);   
            } else if (result == 0) {
                SetNZP(CPU, 2);
            } else if (result > 0) {
                SetNZP(CPU, 1);
            }
            
            WriteOut(CPU, output);
            
            break;
        default : printf("Invalid pairing - has to stop and fail\n");
            //return -1;
               
    }  

    return 0;
	
}


//////////////// PARSING HELPER FUNCTIONS ///////////////////////////

/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState* CPU, FILE* output) {
    
    unsigned short int bigOpp;
    unsigned short int inst;
    unsigned short int dummy;
    int msb;
    
    inst = CPU->memory[CPU->PC];
    
    bigOpp = INSN_15_9(inst);
    
    if (bigOpp == 0) {
        CPU->PC = CPU->PC + 1;
    } else if (bigOpp == 4) {
        if (CPU->PSR == 0x0004 || CPU->PSR == 0x8004) {
            dummy = inst;
            dummy = inst & 0x01FF;
            msb = dummy >> 8;
            
            if (msb == 1) {
                dummy = dummy + 0xFE00;
            }
            CPU->PC = CPU->PC + 1 + dummy;
        } else {
            CPU->PC = CPU->PC + 1;
        }
    } else if (bigOpp == 6) {
        if (CPU->PSR == 0x0004 || CPU->PSR == 0x8004 ||
           CPU->PSR == 0x0002 || CPU->PSR == 0x8002) {
            dummy = inst;
            dummy = inst & 0x01FF;
            msb = dummy >> 8;
            
            if (msb == 1) {
                dummy = dummy + 0xFE00;
            }
            CPU->PC = CPU->PC + 1 + dummy;
        } else {
            CPU->PC = CPU->PC + 1;
        }
        
    } else if (bigOpp == 5) {
        if (CPU->PSR == 0x0004 || CPU->PSR == 0x8004 ||
           CPU->PSR == 0x0001 || CPU->PSR == 0x8001) {
            dummy = inst;
            dummy = inst & 0x01FF;
            msb = dummy >> 8;
            
            if (msb == 1) {
                dummy = dummy + 0xFE00;
            }
            CPU->PC = CPU->PC + 1 + dummy;
        } else {
            CPU->PC = CPU->PC + 1;
        }
        
    } else if (bigOpp == 2) {
        if (CPU->PSR == 0x0002 || CPU->PSR == 0x8002 ) {
            dummy = inst;
            dummy = inst & 0x01FF;
            msb = dummy >> 8;
            
            if (msb == 1) {
                dummy = dummy + 0xFE00;
            }
            CPU->PC = CPU->PC + 1 + dummy;
        } else {
            CPU->PC = CPU->PC + 1;
        }
        
    } else if (bigOpp == 3) {
        if (CPU->PSR == 0x0002 || CPU->PSR == 0x8002 ||
           CPU->PSR == 0x0001 || CPU->PSR == 0x8001 ) {
            dummy = inst;
            dummy = inst & 0x01FF;
            msb = dummy >> 8;
            
            if (msb == 1) {
                dummy = dummy + 0xFE00;
            }
            CPU->PC = CPU->PC + 1 + dummy;
        } else {
            CPU->PC = CPU->PC + 1;
        }
        
    } else if (bigOpp == 1) {
        if (CPU->PSR == 0x0001 || CPU->PSR == 0x8001) {
            dummy = inst;
            dummy = inst & 0x01FF;
            msb = dummy >> 8;
            
            if (msb == 1) {
                dummy = dummy + 0xFE00;
            }
            CPU->PC = CPU->PC + 1 + dummy;
        } else {
            CPU->PC = CPU->PC + 1;
        }
        
    } else if (bigOpp == 7) {
        if (CPU->PSR == 0x0002 || CPU->PSR == 0x8002 ||
           CPU->PSR == 0x0001 || CPU->PSR == 0x8001 ||
            CPU->PSR == 0x0004 || CPU->PSR == 0x8004) {
            dummy = inst;
            dummy = inst & 0x01FF;
            msb = dummy >> 8;
            
            if (msb == 1) {
                dummy = dummy + 0xFE00;
            }
            CPU->PC = CPU->PC + 1 + dummy;
        } 
    } 

    WriteOut(CPU, output);
}
        
/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState* CPU, FILE* output) {
    
    unsigned short int inst;
    unsigned short int subopp;
    unsigned short int destReg;
    unsigned short int srcReg;
    unsigned short int rtReg;
	unsigned short int addImm;
	signed short int x;
    int v;
    unsigned short int result;
	
	CPU->NZP_WE = 1;
    CPU->regFile_WE = 1;
	
	CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    inst = CPU->memory[CPU->PC];
    subopp = INSN_SUB_OP(inst);
	
	destReg = INSN_11_9(inst);
    srcReg = INSN_8_6(inst);
    rtReg = INSN_2_0(inst);
	
	// which register that is being stored in 
	whichRegister = destReg;
    
    if (subopp == 0) {
        CPU->R[destReg] = CPU->R[srcReg] + CPU->R[rtReg];
        
    } else if (subopp == 1) {
		CPU->R[destReg] = CPU->R[srcReg] * CPU->R[rtReg];
        
	} else if (subopp == 2) {
		CPU->R[destReg] = CPU->R[srcReg] - CPU->R[rtReg];
        
	} else if (subopp == 3) {
		CPU->R[destReg] = CPU->R[srcReg] / CPU->R[rtReg];
		
	} else if (subopp >= 4) {

		addImm = INSN_4_0(inst);
        x = addImm >> 4;
        if (x == 1) {
            addImm = addImm + 0xFFE0;
        }
        CPU->R[destReg] = CPU->R[srcReg] + addImm;
	} 
 
    result = CPU->R[destReg];
    CPU->regInputVal = result;
    
    // set NZP
    v = result >> 15;
    
    if (v == 1) {
        SetNZP(CPU, 4);   
    } else if (result == 0) {
        SetNZP(CPU, 2);
    } else if (result > 0) {
        SetNZP(CPU, 1);
    }

    WriteOut(CPU, output); 
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState* CPU, FILE* output) {
    unsigned short int inst;
    unsigned short int subopp;
    unsigned short int srcReg;
    unsigned short int rtReg;
    signed short int imm;
    unsigned short int res;
    signed short int a;
    signed short int b;
    unsigned short int p;
    unsigned short int q;
    int x;
    
    inst = CPU->memory[CPU->PC];
    subopp = INSN_8_7(inst);

    srcReg = INSN_11_9(inst);
    rtReg = INSN_2_0(inst);
    
    if (subopp == 0) { // CMP
        a = CPU->R[srcReg];
        b = CPU->R[rtReg];
        
        if (a > b) {
            SetNZP(CPU, 1);
        } else if (a == b) {
            SetNZP(CPU, 2);
        } else if (a < b) {
            SetNZP(CPU, 4);
        }
  
    } else if (subopp == 1) {
        p = CPU->R[srcReg];
        q = CPU->R[rtReg];
        
        if (p > q) {
            SetNZP(CPU, 1);
        } else if (p == q) {
            SetNZP(CPU, 2);
        } else if (p < q) {
            SetNZP(CPU, 4);
        }
    } else if (subopp == 2) {
        a = CPU->R[srcReg];
        imm = inst & 0x007F;
        x = imm >> 6;
        
        if (x == 1) {
            imm = imm & 0x007F;
            imm = imm + 0xFF80;
        }
        
        if (a > imm) {
            SetNZP(CPU, 1);
        } else if (a == imm) {
            SetNZP(CPU, 2);
        } else if (a < imm) {
            SetNZP(CPU, 4);
        }
        
        
    } else if (subopp == 3) {
        p = CPU->R[srcReg];
        q = inst & 0x007F;
        
        if (p > q) {
            SetNZP(CPU, 1);
        } else if (p == q) {
            SetNZP(CPU, 2);
        } else if (p < q) {
            SetNZP(CPU, 4);
        }    
    }
    
    CPU->NZP_WE = 1;
    CPU->rsMux_CTL = 2;
    WriteOut(CPU, output);
    CPU->PC = CPU->PC + 1;   
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState* CPU, FILE* output) {
	
    unsigned short int inst;
    unsigned short int subopp;
    unsigned short int destReg;
    unsigned short int srcReg;
    unsigned short int rtReg;
    unsigned short int andImm;
    unsigned short int result;
    int x;

	CPU->NZP_WE = 1;
    CPU->regFile_WE = 1;

	CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    
    inst = CPU->memory[CPU->PC];
    subopp = INSN_SUB_OP(inst);
    
    destReg = INSN_11_9(inst);
    srcReg = INSN_8_6(inst);
    rtReg = INSN_2_0(inst);
    
    // store which register is being written to
    whichRegister = destReg;
    
    if (subopp == 0) { // AND
        CPU->R[destReg] = CPU->R[srcReg] & CPU->R[rtReg];
        
    } else if (subopp == 1) { // NOT
        CPU->R[destReg] = ~ CPU->R[srcReg];
        
    } else if (subopp == 2) { // OR
        CPU->R[destReg] = CPU->R[srcReg] | CPU->R[rtReg];
        
    } else if (subopp == 3) { // XOR
        CPU->R[destReg] = CPU->R[srcReg] ^ CPU->R[rtReg];
        
    } else if (subopp >= 4) { // AND immediate
        
        andImm = INSN_4_0(inst);
        x = andImm >> 4;
        if (x == 1) {
            andImm = andImm + 0xFFE0;
        }
        CPU->R[destReg] = CPU->R[srcReg] & andImm;   
    }
   
	// some call to setNZP
	result = CPU->R[destReg];
    x = result >> 15;
    
    CPU->NZP_WE = 1;
    
    
    if (x == 1) {
        SetNZP(CPU, 4);
    } else if (result == 0) {
        SetNZP(CPU, 2);
    } else if (result > 0) {
        SetNZP(CPU, 1);
    }

    // regInputVal
    CPU->regInputVal = CPU->R[destReg];

	WriteOut(CPU, output);
    CPU->PC = CPU->PC + 1;
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState* CPU, FILE* output) {
    
    unsigned short int inst;
    unsigned short int subopp;
    unsigned short int srcReg;
	unsigned short int imm;
    unsigned short int dummyPC;
    int x;
    //printf("vig");
    CPU->regFile_WE = 0;

    inst = CPU->memory[CPU->PC];
    subopp = INSN_OP_5(inst);
    printf("%d ", subopp);
    
    if (subopp == 24) { // JMPR Rs
 
        srcReg = INSN_8_6(inst);
        CPU->PC = CPU->R[srcReg];
        
    } else if (subopp == 25) {// JMP IMM11 <LABEL>
        //printf("lol");
        imm = inst & 0x07FF;
        x = imm >> 10;
        
        if (x == 1) {
            imm = imm + 0xF800;
        }
        
        CPU->PC = CPU->PC + 1 + imm;  
    }
    
    // will need to setNZP as well 
    
    WriteOut(CPU, output); 
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState* CPU, FILE* output) {
    
    unsigned short int inst;
    unsigned short int subopp;
    unsigned short int srcReg;
    unsigned short int imm;
    unsigned short int dummyPC;
    unsigned short int result;
    unsigned short int temp;
    int x;
    
    inst = CPU->memory[CPU->PC];
    srcReg = INSN_8_6(inst);
    subopp = INSN_OP_5(inst);
	printf("%d", subopp);
    
    if (subopp == 8) {// JSSR Rs
//         if (srcReg == 7) {
//             temp = CPU->R[7];
//             CPU->R[7] = CPU->PC + 1;
//             CPU->PC = temp;
//         } else {
//             CPU->R[7] = CPU->PC + 1;
//             CPU->PC = CPU->R[srcReg];    
//         }
        temp = CPU->R[srcReg];
		CPU->R[7] = CPU->PC + 1;
		CPU->PC = temp;
        
    } else if (subopp == 9) {// JSSR IMM11 <LABEL>
        
        CPU->R[7] = CPU->PC + 1;
        imm = inst & 0x07FF;
        dummyPC = CPU->PC;
        
        CPU->PC = ((dummyPC & 0x8000) | (imm << 4));
    
    }
    result = CPU->R[7];
    x = result >> 15;
    
    CPU->NZP_WE = 1;
    
    
    if (x == 1) {
        SetNZP(CPU, 4);
    } else if (result == 0) {
        SetNZP(CPU, 2);
    } else if (result > 0) {
        SetNZP(CPU, 1);
    }
    
    //whichRegister
    whichRegister = 7;
    
    CPU->regFile_WE = 1;
    // regInputVal
    CPU->regInputVal = CPU->R[7];

    WriteOut(CPU, output);  
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState* CPU, FILE* output) {
    
    unsigned short int inst;
    unsigned short int subopp;
    unsigned short int destReg;
    unsigned short int srcReg;
    unsigned short int rtReg;
	unsigned short int shiftImm;
	unsigned short int x;
    unsigned short int result;
    signed short int lol;
    int y;
    
    inst = CPU->memory[CPU->PC];
    subopp = INSN_5_4(inst);
    
    destReg = INSN_11_9(inst);
    srcReg = INSN_8_6(inst);
    
    CPU->regFile_WE = 1;
    
    
    if (subopp == 0) {
        
        shiftImm = inst & 0xF;
        CPU->R[destReg] = CPU->R[srcReg] << shiftImm;
        
    } else if (subopp == 1) {
        
        shiftImm = inst & 0xF;
        lol = CPU->R[srcReg];
        lol = lol >> shiftImm;
        CPU->R[destReg] = lol;
           
    } else if (subopp == 2) {
        shiftImm = inst & 0xF;
        CPU->R[destReg] = CPU->R[srcReg] >> shiftImm;
        
    } else if (subopp == 3) {
        
        rtReg = INSN_2_0(inst);
        CPU->R[destReg] = CPU->R[srcReg] % CPU->R[rtReg];
    }
    // regInputVal
    CPU->regInputVal = CPU->R[destReg];
    
    // whichRegister
    whichRegister = destReg;
    
    CPU->NZP_WE = 1;
    
    result = CPU->R[destReg];
    x = result >> 15;
    
    if (x == 1) {
        SetNZP(CPU, 4);
    } else if (result == 0) {
        SetNZP(CPU, 2);
    } else if (result > 0) {
        SetNZP(CPU, 1);
    }

    WriteOut(CPU, output);
    CPU->PC = CPU->PC + 1;      
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState* CPU, short result) {
    
    unsigned short int MSB;
    int x;
    
    CPU->NZPVal = result;
    
    MSB = CPU->PSR;
    x = MSB >> 15;
    
    if (result == 0) {
        if (x == 1) {
            CPU->PSR = 0x8000;
        } else if (x == 0) {
            CPU->PSR = 0x0000;
        } 
    } else if (result == 1) {
        if (x == 1) {
            CPU->PSR = 0x8001;
        } else if (x == 0) {
            CPU->PSR = 0x0001;
        }
    } else if (result == 2) {
        if (x == 1) {
            CPU->PSR = 0x8002;
        } else if (x == 0){
            CPU->PSR = 0x0002;
        } 
    } else if (result == 4) {
        if (x == 1) {
            CPU->PSR = 0x8004;
        } else if (x == 0) {
            CPU->PSR = 0x0004;
        }
    }    
}
