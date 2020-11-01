/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU) {
    
	FILE* src;
	int i;
	int v1;
	int v2;
	int v3;
    int v4;    // variable within subloops
    int v5;    // variable within subloops
    int v6;
	int last_header;
    int num_instr;
    int address;
    
    src = fopen(filename, "rb");
	
	while(!feof(src)) {
		v1 = fgetc(src);
		v2 = fgetc(src);
		
		v3 = combineBytes(v1, v2);
		
		if (v3 == 0xCADE || v3 == 0xDADA) {
			last_header = v3;
            v4 = fgetc(src);
            v5 = fgetc(src);

            address = combineBytes(v4, v5);
            
            v1 = fgetc(src);
            v2 = fgetc(src);

            num_instr = combineBytes(v1, v2);

            for (i = address; i < address + num_instr; i++) {
                v1 = fgetc(src);
                v2 = fgetc(src);
                CPU->memory[i] = combineBytes(v1, v2);
                //print statements for regrade testing
                //printf("%04X ", i);
                //printf("%04X ", CPU->memory[i]);
            }        
		} else if (v3 == 0xC3B7) {
            //printf("found label\n");
            // address - we don't need it
            v1 = fgetc(src);
            v2 = fgetc(src);
            
            // number of chars
            v4 = fgetc(src);
            v5 = fgetc(src);
            v6 = combineBytes(v4, v5);
            
            for (i = 0; i < v6; i++) {
                v4 = fgetc(src);
            }
            
        } else if (v3 == 0xF17E) {
            printf("found something");
            // address - we don't need it
            v1 = fgetc(src);
            v2 = fgetc(src);
            
            // number of chars - need to skip ahead
            v4 = fgetc(src);
            v5 = fgetc(src);
            v6 = combineBytes(v4, v5);
            
            for (i = 0; i < v6; i++) {
                v4 = fgetc(src);
             }
            
        } else if (v3 == 0x175E) {
            printf("haha");
            v4 = fgetc(src);
            v5 = fgetc(src);
            
            v4 = fgetc(src);
            v5 = fgetc(src);
            
            v4 = fgetc(src);
            v5 = fgetc(src);
            
            v4 = fgetc(src);
            v5 = fgetc(src);   
        }
    }
	fclose(src);
    return 0;   
}

int combineBytes(int a, int b) {
    int c;
    a = a << 8;
    c = a | b;
    return c;
}

  
