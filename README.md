# multithread-tcp-server
# Author Maria Contreras
 This is a thread-per-client TCP server that:
   Accepts multiple client connections.
   Spawns one thread per client
   Recieves commands from each client
   Executes commands with system()
   Sends back only the success/failure status, not the command output.
   Continues Serving clients until they disconnect

 I created this program to demontrate my ability to:
   - Understand TCP client‑server communication.
   - Use sockets and POSIX threads.
   - Execute system commands safely.
   - Manage concurrency and thread lifecycles.
   - Handle network and threading errors robustly.
