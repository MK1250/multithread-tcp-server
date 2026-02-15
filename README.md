# multithread-tcp-server
# Author: Maria Contreras
 This project is an implementation of a multi-thread TCP server in C using POSIX
 threads (pthreads). The server handles multiple simultaneous clients, creating one
 thread per client connection, and execute commands sent by each client using
 system().
 - Accepts multiple client connections.
 - Spawns one thread per client
 - Recieves commands from each client
 - Executes commands with system()
 - Sends back only the success/failure status, not the command output.
 - Continues Serving clients until they disconnect

 I created this program to demontrate my ability to:
   - Understand TCP client‑server communication.
   - Use sockets and POSIX threads.
   - Execute system commands safely.
   - Manage concurrency and thread lifecycles.
   - Handle network and threading errors robustly.
