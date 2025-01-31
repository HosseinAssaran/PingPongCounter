# Process Communication via Sahred Memory

This project implements two separate processes that communicate using Sahred Memory. The first process (Initiator) sends a numerical counter to the second process (Receiver), which increments and returns it. The exchange continues until the counter reaches 10, at which point both processes terminate.

## Features
- **Interprocess Communication (IPC)** using Sahred Memory
- **Bidirectional message passing** between processes
- **Process synchronization** using semaphore
- **Logging** to both console and `program_log.txt`
- **CMake-based build system** for easy compilation on Linux (tested on Ubuntu 22.04)

## Compilation and Execution Instructions

### **1. Install Dependencies**
Ensure you have `cmake` and `g++` installed:
```sh
sudo apt update
sudo apt install cmake g++
```

### **2. Clone and Navigate to the Project Directory**
```sh
git clone https://github.com/HosseinAssaran/PingPongCounter.git
cd PingPongCounter
```

### **3. Build the Project**
```sh
mkdir build && cd build
cmake ..
make
```

### **4. Running the Program**
The program consists of two processes:
1. **Initiator Process (`initiator`)**: Starts the counter and communicates with the receiver.
2. **Receiver Process (`receiver`)**: Receives the counter, increments it, and sends it back.

### Step 1: Start the Initiator
```bash
./initiator
```

### Step 2: Start the Receiver (in another terminal)
```bash
./receiver
```
### **5. Check the Logs**
Execution logs are saved in `program_log.txt`. You can view them with:
```sh
cat process_log.txt
```

Both processes will exchange numbers, logging each step to `stdout` and `program_log.txt`. Once the counter reaches **10**, both processes terminate automatically.

### **6. Log Output**
The log file (`program_log.txt`) records all interactions:
```txt
Initiator sends value: 0
Receiver send value: 1
Initiator sends value: 2
...
Initiator process finished. Counter reached 10.
Receiver process finished. Counter reached 10.
```

### **7. Cleanup**
Once the program terminates, clean up shared memory and semaphores (if needed):
```bash
ipcs -m  # Check active shared memory segments
ipcrm -M 0x<shm_id>  # Remove shared memory (replace <shm_id> with the correct ID)

ls /dev/shm  # Check active shared memory objects
rm /dev/shm/shared_counter  # Remove manually if necessary
```

## Notes
- If execution fails, verify that `program_log.txt` contains error details.
