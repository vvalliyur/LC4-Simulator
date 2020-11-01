/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"

// Global variable defining the current state of the machine
MachineState* CPU;

int main(int argc, char** argv) {
	int k;
    int i;
	char* filename = NULL;
    char* outfile = NULL;
    unsigned long l;
    int r;
    FILE* writefile;
	
    CPU = malloc(sizeof(MachineState));
    memset(CPU->memory, 0, 65536 * 2);
    
    Reset(CPU);
    
// 	// check argc and argv
//     if (argc < 2) {
//         printf("Incorrect number of arguments provided");
//         return -1;
//     } else {
//         for (i = 2; i < argc; i++) {
//             filename = argv[i];
//             k = ReadObjectFile(filename, CPU);
//         }
//     }
//     
    if (argc < 2) {
        printf("Incorrect number of arguments\n");
        return -1;
    }
    
    for (i = 2; i < argc; i++) {
        filename = argv[i];
        k = ReadObjectFile(filename, CPU);
    }
    
    
    outfile = argv[1];
    writefile = fopen(outfile, "w");
    
    //for (i = 0; i < 65536; i++) {
    //    if (CPU->memory[i] != 0) {
    //        fputs("address: ", writefile);
    //        fprintf(writefile, "%05d ", i);
    //        fputs("contents: ", writefile);
    //        fprintf(writefile, "0x%04X ", CPU->memory[i]);
    //        fputs("\n", writefile);
    //    }
    //}
    //
    //for (i = 0; i < 1900; i++) {
    //    UpdateMachineState(CPU, writefile);
    //}
    

    while (CPU->PC != 0x80FF) {
        r = UpdateMachineState(CPU, writefile);
        if (r == 1 || r == 2 || r == 3
           || r == 4) {
            break;
        }
    }
        
	fclose(writefile);
    free(CPU);
	return 0;	
}

