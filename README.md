# Process Communication via Unix Domain Sockets

This project implements two separate processes that communicate using Unix domain sockets. The first process (Initiator) sends a numerical counter to the second process (Receiver), which increments and returns it. The exchange continues until the counter reaches 10, at which point both processes terminate.

## Features
- **Interprocess Communication (IPC)** using Unix domain sockets
- **Bidirectional message passing** between processes
- **Process synchronization** using structured communication
- **Logging** to both console and `process_log.txt`
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

### **4. Run the Program**
```sh
./pingpongcounter
```

### **5. Check the Logs**
Execution logs are saved in `process_log.txt`. You can view them with:
```sh
cat process_log.txt
```

## Code Testing
To test the execution, ensure the expected log output is produced:
1. The initiator starts and sends `1` to the receiver.
2. The receiver increments and returns `2`.
3. This exchange continues until `10` is reached.
4. Both processes terminate successfully.

## Cleanup
After execution, remove the Unix domain socket file if necessary:
```sh
rm -f /tmp/process_communication.sock
```

## Notes
- If the program does not start correctly, ensure that no previous socket file exists:
  ```sh
  rm -f /tmp/process_communication.sock
  ```
- If execution fails, verify that `process_log.txt` contains error details.
