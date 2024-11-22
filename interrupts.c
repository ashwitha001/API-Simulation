#include "interrupts_101186335.h"


// Function that reads data from external_file.txt and stores it in an array
void readExternalFiles(char* filename, ExternalFile files[], int* count) {
    FILE* file = fopen(filename, "r"); //opens file in read mode
    if (!file) { //checks if file opened successfully
        fprintf(stderr, "Error opening external files list: %s\n", filename);
        exit(1);
    }

    int i = 0;
    while (fscanf(file, " %49[^,], %d", files[i].name, &files[i].size) == 2) {
        i++;
        if (i >= MAX_FILES) {
            fprintf(stderr, "Maximum file count reached.\n");
            break;
        }
    }
    *count = i; //stores number of files read in variable count
    fclose(file);
}

//Function that reads and parces through the trace file
void readFile(char* filename, TraceEvent trace[], int* size) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening trace file: %s\n", filename);
        exit(1);
    }

    int i = 0;
    while (fscanf(file, " %49[^,], %d", trace[i].type, &trace[i].duration) == 2) {
        i++;
        if (i >= MAX_TRACE) {
            fprintf(stderr, "Maximum trace size reached.\n");
            break;
        }
    }
    *size = i; //number of trace events stored in size variable

    fclose(file);
}

//Function that handles FORK sys call
void handleFork(int* time, FILE* output, PCB* pcbTable, int* pcbCount, int* nextPID) {
    fprintf(output, "%d, %d, switch to kernel mode\n", *time, 1);
    *time += 1;

    fprintf(output, "%d, %d, context saved\n", *time, 3);
    *time += 3;

    fprintf(output, "%d, %d, find vector 2 in memory position 0x0004\n", *time, 1);
    *time += 1;

    fprintf(output, "%d, %d, load address 0X0695 into the PC\n", *time, 1);
    *time += 1;

    // Copy parent PCB to child PCB and assign it a new PID
    PCB childPCB = pcbTable[0]; //copy initial pcb
    childPCB.pid = (*nextPID)++; //assign unique pid to child pcb
    pcbTable[(*pcbCount)++] = childPCB;

    fprintf(output, "%d, %d, FORK: copy parent PCB to child PCB\n", *time, 4);
    *time += 4;

    fprintf(output, "%d, %d, scheduler called\n", *time, 16);
    *time += 16;

    fprintf(output, "%d, %d, IRET\n", *time, 1);
    *time += 1;
}

// Function that handles the EXEC sys call
void handleExec(int* time, FILE* output, const char* programName, int programSize, PCB* pcbTable, Partition memoryPartitions[], int pcbIndex) {
    fprintf(output, "%d, %d, switch to kernel mode\n", *time, 1);
    *time += 1;

    fprintf(output, "%d, %d, context saved\n", *time, 3);
    *time += 3;

    fprintf(output, "%d, %d, find vector 3 in memory position 0x0006\n", *time, 1);
    *time += 1;

    fprintf(output, "%d, %d, load address 0X042B into the PC\n", *time, 1);
    *time += 1;

    fprintf(output, "%d, %d, EXEC: load %s of size %dMb\n", *time, 12, programName, programSize);
    *time += 12;

    // Loop through memory partitions to find one that fits program size
    for (int i = 0; i < 6; i++) {
        if (!memoryPartitions[i].occupied && memoryPartitions[i].size >= programSize) {
            memoryPartitions[i].occupied = true;
            strncpy(memoryPartitions[i].code, programName, sizeof(memoryPartitions[i].code) - 1);
            pcbTable[pcbIndex].partitionNumber = i + 1;
            pcbTable[pcbIndex].size = programSize;

            fprintf(output, "%d, %d, found partition %d with %dMb of space\n", *time, 22, memoryPartitions[i].number, memoryPartitions[i].size);
            *time += 22;

            fprintf(output, "%d, %d, partition %d marked as occupied\n", *time, 6, memoryPartitions[i].number);
            *time += 6;

            fprintf(output, "%d, %d, updating PCB with new information\n", *time, 17);
            *time += 17;
            break;
        }
    }

    fprintf(output, "%d, %d, scheduler called\n", *time, 3);
    *time += 3;

    fprintf(output, "%d, %d, IRET\n", *time, 1);
    *time += 1;
}

void run(TraceEvent event, int* time, FILE* output, const VectorTable vector_table[], ExternalFile externalFiles[], int fileCount, PCB* pcbTable, int* pcbCount, int* nextPID, Partition memoryPartitions[]) {
    //If the event type is "FORK", handle FORK call. 
    if (strcmp(event.type, "FORK") == 0) {
        handleFork(time, output, pcbTable, pcbCount, nextPID);
        //if event type is "EXEC", handle EXEC call
    } else if (strncmp(event.type, "EXEC", 4) == 0) {
        char programName[50];
        int programSize = 0;
        sscanf(event.type, "EXEC %s", programName);
        //search external files for program to find program size
        for (int i = 0; i < fileCount; i++) {
            if (strcmp(externalFiles[i].name, programName) == 0) {
                programSize = externalFiles[i].size;
                break;
            }
        }
        handleExec(time, output, programName, programSize, pcbTable, memoryPartitions, *pcbCount - 1);
        //If the event type is "CPU", start CPU execution.
    } else if (strcmp(event.type, "CPU") == 0) {
        //CPU execution, increment the time by duration of event
        fprintf(output, "%d, %d, CPU execution\n", *time, event.duration);
        *time += event.duration;
    } else if (strncmp(event.type, "SYSCALL", 7) == 0) {
        int interrupt_num;
        if (sscanf(event.type, "SYSCALL %d", &interrupt_num) == 1) {
            if (interrupt_num >= 0 && interrupt_num < VECTOR_TABLE_SIZE) {
                fprintf(output, "%d, %d, switch to kernel mode\n", *time, 1);
                *time += 1;

                fprintf(output, "%d, %d, context saved\n", *time, 3);
                *time += 3;

                char find_vector_msg[50];
                snprintf(find_vector_msg, sizeof(find_vector_msg), 
                         "find vector %d in memory position 0x%04X", interrupt_num, interrupt_num * 2);
                fprintf(output, "%d, %d, %s\n", *time, 1, find_vector_msg);
                *time += 1;

                char load_isr_msg[50];
                snprintf(load_isr_msg, sizeof(load_isr_msg), 
                         "load address 0x%04X into the PC", vector_table[interrupt_num].address);
                fprintf(output, "%d, %d, %s\n", *time, 1, load_isr_msg);
                *time += 1;

                fprintf(output, "%d, %d, SYSCALL: run the ISR\n", *time, 230);
                *time += 47;

                fprintf(output, "%d, %d, transfer data\n", *time, 110);
                *time += 57;

                fprintf(output, "%d, %d, check for errors\n", *time, 50);
                *time += 15;

                fprintf(output, "%d, %d, IRET\n", *time, 1);
                *time += 1;
            } else {
                fprintf(stderr, "Invalid SYSCALL interrupt number: %d\n", interrupt_num);
            }
        }
    } else if (strncmp(event.type, "END_IO", 6) == 0) {
        int interrupt_num;
        //get interrupt num from event type
        if (sscanf(event.type, "END_IO %d", &interrupt_num) == 1) {
            if (interrupt_num >= 0 && interrupt_num < VECTOR_TABLE_SIZE) {
                fprintf(output, "%d, %d, check priority of interrupt\n", *time, 1);
                *time += 1;

                fprintf(output, "%d, %d, check if masked\n", *time, 1);
                *time += 1;

                fprintf(output, "%d, %d, switch to kernel mode\n", *time, 1);
                *time += 1;

                fprintf(output, "%d, %d, context saved\n", *time, 3);
                *time += 3;

                char find_vector_msg[50];
                snprintf(find_vector_msg, sizeof(find_vector_msg), 
                         "find vector %d in memory position 0x%04X", interrupt_num, interrupt_num * 2);
                fprintf(output, "%d, %d, %s\n", *time, 1, find_vector_msg);
                *time += 1;

                char load_isr_msg[50];
                snprintf(load_isr_msg, sizeof(load_isr_msg), 
                         "load address 0x%04X into the PC", vector_table[interrupt_num].address);
                fprintf(output, "%d, %d, %s\n", *time, 1, load_isr_msg);
                *time += 1;

                fprintf(output, "%d, %d, END_IO\n", *time, 248);
                *time += 248;

                fprintf(output, "%d, %d, IRET\n", *time, 1);
                *time += 1;
            } else {
                fprintf(stderr, "Invalid END_IO interrupt number: %d\n", interrupt_num);
            }
        }
    }

}

// Start simulation by initializing data and running each trace event
void startSimulation(TraceEvent trace[], int size, FILE* output_file, VectorTable vector_table[VECTOR_TABLE_SIZE], ExternalFile externalFiles[], int fileCount) {
    int time = 0;
    int nextPID = 11;
    int pcbCount = 1;
    PCB pcbTable[MAX_TRACE] = { { nextPID++, "init", 6, 1, 0, true, 0 } };
    Partition memoryPartitions[6] = { {1, 40, "free", false}, {2, 25, "free", false}, {3, 15, "free", false}, {4, 10, "free", false}, {5, 8, "free", false}, {6, 2, "init", true} }; // Define memory partitions

    for (int i = 0; i < size; i++) {
        run(trace[i], &time, output_file, vector_table, externalFiles, fileCount, pcbTable, &pcbCount, &nextPID, memoryPartitions);
        saveSystemStatus(time, pcbTable, pcbCount, memoryPartitions);  // Log status after each event
    }
}

// Save system status including PCB into system_status.txt file
void saveSystemStatus(int time, PCB* pcbTable, int pcbCount, Partition* memoryPartitions) {
    FILE* file = fopen("system_status_101186335_101187120.txt", "a");
    fprintf(file, "!-----------------------------------------------------------!\n");
    fprintf(file, "Save Time: %d ms\n", time);
    fprintf(file, "+--------------------------------------------+\n");
    fprintf(file, "| PID | Program Name | Partition Number | Size |\n");
    fprintf(file, "+--------------------------------------------+\n");

    for (int i = 0; i < pcbCount; i++) {
        fprintf(file, "| %3d | %11s | %15d | %4d |\n", pcbTable[i].pid, pcbTable[i].programName, pcbTable[i].partitionNumber, pcbTable[i].size);
    }
    fprintf(file, "+--------------------------------------------+\n");
    fprintf(file, "!-----------------------------------------------------------!\n");
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "%s <trace_file> <external_files> <output_file>\n", argv[0]);
        return 1;
    }

    char *trace_filename = argv[1]; //tracefile name
    char *external_filename = argv[2]; //external file name
    char *output_filename = argv[3]; //output file name

    TraceEvent trace[MAX_TRACE];
    ExternalFile externalFiles[MAX_FILES];
    int traceSize, fileCount; //size of trace and external file

    readTraceFile(trace_filename, trace, &traceSize);
    readExternalFiles(external_filename, externalFiles, &fileCount);

    FILE *output_file = fopen(output_filename, "w");
    VectorTable vector_table[VECTOR_TABLE_SIZE] = {
        { 0, 0x01E3 }, { 1, 0x029C }, { 2, 0x0695 }, { 3, 0x042B },
        { 4, 0x0292 }, { 5, 0x048B }, { 6, 0x0639 }, { 7, 0x00BD },
        { 8, 0x06EF }, { 9, 0x036C }, { 10, 0x07B0 }, { 11, 0x01F8 },
        { 12, 0x03B9 }, { 13, 0x06C7 }, { 14, 0x0165 }, { 15, 0x0584 },
        { 16, 0x02DF }, { 17, 0x05B3 }, { 18, 0x060A }, { 19, 0x0765 },
        { 20, 0x07B7 }, { 21, 0x0523 }, { 22, 0x03B7 }, { 23, 0x028C },
        { 24, 0x05E8 }, { 25, 0x05D3 }
    };

    startSimulation(trace, traceSize, output_file, vector_table, externalFiles, fileCount);

    fclose(output_file);
    return 0;
}
