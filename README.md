# API Simulation

Simulates a CPU execution system with interrupt handling, system calls, and process scheduling. The simulator generates logs of all system activities, including memory allocation, process forking, and execution.

---

## **Files in the Project**

### **Source Code**
- **`interrupts.h`**: Header file
- **`interrupts.c`**: Implementation of the simulation logic, including trace file reading, event handling, and output logging.

### **Input Files**
- **`trace.txt`**: Includes the sequence of events (e.g., FORK, EXEC, CPU execution) with relevant durations and details.
- **`external_files.txt`**: Lists programs and their memory sizes.
- **`program#.txt`**: Provides instructions for specific programs loaded into memory during execution.

### **Output Files**
- **`execution.txt`**: Logs events with timestamps, durations, and descriptions.
- **`system_status.txt`**: Saves the current state of the system after each system call.


## **License**
This project is licensed under the MIT License.
