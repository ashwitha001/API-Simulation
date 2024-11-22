#ifndef INTERRUPTS_H_101186335_101187120
#define INTERRUPTS_H_101186335_101187120

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRACE 100
#define MAX_FILES 10
#define VECTOR_TABLE_SIZE 26

// Structure for trace events
typedef struct {
    char type[50];
    int duration;
} TraceEvent;

// Structure for external program files
typedef struct {
    char name[50];
    int size;
} ExternalFile;

// Structure for vector table entries
typedef struct {
    int interrupt_num;
    int address;
} VectorTable;

// Structure for memory partitions
typedef struct {
    int number;
    int size;
    char code[10];  // Stores status like "free", "init", or program name
    int occupied;
} Partition;

// Structure for the PCB
typedef struct {
    int pid;
    char programName[20];
    int partitionNumber;
    int size;
    int remainingCPUTime;
    int isReady;
    int parentPID;
} PCB;

// Function declarations
void readExternalFiles(char* filename, ExternalFile files[], int* count);
void readTraceFile(char* filename, TraceEvent trace[], int* size);
void run(TraceEvent event, int* time, FILE* output, const VectorTable vector_table[], ExternalFile externalFiles[], int fileCount, PCB* pcbTable, int* pcbCount, int* nextPID, Partition memoryPartitions[]);
void handleFork(int* time, FILE* output, PCB* pcbTable, int* pcbCount, int* nextPID);
void handleExec(int* time, FILE* output, const char* programName, int programSize, PCB* pcbTable, Partition memoryPartitions[], int pcbIndex);
void startSimulation(TraceEvent trace[], int size, FILE* output_file, VectorTable vector_table[], ExternalFile externalFiles[], int fileCount);
void saveSystemStatus(int time, PCB* pcbTable, int pcbCount, Partition* memoryPartitions);

#endif
