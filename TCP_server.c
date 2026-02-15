/*
 * Student Implementation Template
 *
 * REQUIREMENTS:
 * - Create TCP server socket and bind to port 8889
 * - Accept multiple client connections in a loop until shutdown
 * - Create ONE thread to handle all commands from each client
 * - Thread executes commands using system() and sends status back
 * - Proper thread cleanup using pthread_detach()
 * - Handle edge cases (socket errors, thread creation failure, client disconnection)
 */

#include "TCP_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#include <pthread.h>

/*=======================================================================*/

/**
 * Structure to pass client information to thread
 */
typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_thread_args_t;

/*=======================================================================*/

/**
 * Thread function that handles all requests from a single client
 */
void* handle_client_thread(void* arg)
{
    /* Cast arg to client_thread_args_t */
    client_thread_args_t* args = (client_thread_args_t*)arg;
    
    // TODO: Extract client socket and client address from `args`
    int client_socket = args->client_socket;
    struct sockaddr_in client_addr = args->client_addr;

    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    char status_msg[2048];

    // TODO: Convert client IP to string using inet_ntop()
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, sizeof(client_ip));

    // TODO: Declare `int client_port` and initialize it using ntohs(client_addr.sin_port)
    int client_port = ntohs(client_addr.sin_port);

    printf("Server: Connected to client %s:%d\n", client_ip, client_port);
    fflush(stdout);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // TODO: Receive command from client using recv()
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        // TODO: Check if recv() failed or client disconnected (bytes_received <= 0)
        // - If recv() failed and shutdown was not requested, perror() and break
        // - If client disconnected (bytes_received == 0), print message and break
        if (bytes_received <= 0) {
            // Check if recv() failed due to error
            if (bytes_received < 0) {
                perror("recv");
            }
            break;
        }

        // TODO: Null-terminate string and remove newline
        buffer[bytes_received] = '\0'; // Null-terminate
        buffer[strcspn(buffer, "\r\n")] = '\0'; // Remove newline

        // TODO: Check for empty command and skip (continue) if empty
        if (strlen(buffer) == 0) {
            continue;
        }

        printf("Server: Received command: %s\n", buffer);
        fflush(stdout);
        
        printf("Server: Executing command: %s\n", buffer);
        fflush(stdout);

        // TODO: Execute the command using system() and store the result.
        // The result will be 0 if the command executes successfully; otherwise, it indicates failure.
        int result = system(buffer);


       // TODO: Create a status message indicating whether the command completed successfully.
       // Format: "'%s' completed successfully\n" or "'%s' failed\n"
       // HINT: Use snprintf(), e.g.: snprintf(status_msg, sizeof(status_msg), "'%s' failed\n", buffer);
       if (result == 0) {
            snprintf(status_msg, sizeof(status_msg), "'%s' completed successfully\n", buffer);
       } else {
            snprintf(status_msg, sizeof(status_msg), "'%s' failed\n", buffer);
       }

        // TODO: Send success/failure status message to client using send()
        send(client_socket, status_msg, strlen(status_msg), 0);

        printf("Server: Command completed\n");
        fflush(stdout);
    }
    send(client_socket, status_msg, strlen(status_msg), 0);

    printf("Server: Client %s:%d disconnected\n", client_ip, client_port);
    fflush(stdout);
    
    // TODO: Close client socket
    close(client_socket);

    // TODO: free `args`  ⚠️ Important to prevent memory leak
    free(args);
    
    return NULL;
}

/*=======================================================================*/

/**
 * Main TCP server implementation
 */
int run_tcp_server(void)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    pthread_t thread_id;

    
    // TODO: Create a TCP socket.
    // Example: server_fd = socket();
    // Refer to the socket() man page for parameter details.
    // Check for failure — socket() returns -1 on error, and it should be handled appropriately.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return -1;
    } 

    // TODO: Set socket options using setsockopt(SO_REUSEADDR) to allow address reuse.
    // Example: setsockopt();
    // Refer to the socket() man page for parameter details.
    // This prevents the "Address already in use" error when restarting the server.
    // Always check the return value of setsockopt(); it returns -1 on failure and should be handled appropriately.
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    // TODO: Configure server address structure `server_addr`
    // - Set address family to AF_INET (IPv4)
    // - Bind to all local network interfaces (INADDR_ANY)
    // - Set port number to SERVER_PORT (8889) using network byte order -> htons()
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    server_addr.sin_port = htons(SERVER_PORT); // Set port number in network byte order


    // TODO: Bind the server socket to the local address (SERVER_PORT 8889) using bind()
    // - Use the initialized `server_addr` structure
    // - Refer to the bind() man page for parameter details
    // - Handle failure: bind() returns -1 on error
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }


    // TODO: Listen for incoming connections using listen()
    // - Use the server socket file descriptor
    // - Refer to the listen() man page for parameter details
    // - Handle failure: listen() returns -1 on error
    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }


    printf("Server: Listening on port %d...\n", SERVER_PORT);
    printf("Server: Waiting for client connection...\n");
    fflush(stdout);

   /********************************************************************************************
   `shutdown_requested` is an extern volatile sig_atomic_t variable used for graceful shutdown.   
        How it works:
        - Defined in TCP_server_driver.c and initialized to 0
        - Declared as extern in TCP_server.h for access across files  
        - Set to 1 by signal_handler() when SIGINT (Ctrl+C) or SIGTERM received
        - Checked in TCP_server.c main loop: while (!shutdown_requested)
        
        Why each keyword matters:
        - 'volatile': Tells compiler variable can change unexpectedly (by signal handler),
            preventing optimization that would cache the value and miss the signal
        - 'sig_atomic_t': POSIX atomic type safe for signal handlers, prevents race conditions
        - 'extern': Allows this file to access variable defined in TCP_server_driver.c
   
        This pattern enables safe communication between signal handlers and main program.
        See TCP_server.h and TCP_server_driver.c for complete implementation. 
    ********************************************************************************************/
   
    while (!shutdown_requested) {        
        // TODO: Accept client connection using accept()
        // - Use the server socket file descriptor
        // - Save the client details in `client_addr` 
        // - Refer to the accept() man page for parameter details
        // - Handle failure: accept() returns -1 on error
        // - If accept() failed and shutdown was not requested, perror() and continue
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd < 0) {
            if (shutdown_requested) {
                break; // Exit loop if shutdown was requested
            }
            perror("accept");
            continue; // Continue to next iteration to accept new connection
        }



        // TODO: Allocate memory for thread arguments using malloc()
        // - if malloc() failed, (perror, close client_fd, continue)
        client_thread_args_t* args = malloc(sizeof(client_thread_args_t));
        if (args == NULL) {
            perror("malloc");
            close(client_fd);
            continue;
        }


        // TODO: Set args->client_socket and args->client_addr
        args->client_socket = client_fd;
        args->client_addr = client_addr;


        // 💡 TODO: Create thread using pthread_create(). ⚠️ DO NOT use pthread_join()
        // - Handle failure: pthread_create() returns non-zero on error
        // - if pthread_create() failed, (free args, close client_fd, continue)
        if (pthread_create(&thread_id, NULL, handle_client_thread, (void*)args) != 0) {
            perror("pthread_create");
            free(args);
            close(client_fd);
            continue;
        }
       

        // TODO: Detach thread using pthread_detach()
        // - This allows resources to be automatically reclaimed when thread finishes
        // - Handle failure: pthread_detach() returns non-zero on error
        if (pthread_detach(thread_id) != 0) {
            perror("pthread_detach");
            // Even if detach fails, we should not terminate the server, so we just print an
            // error message and continue. The thread will still run, but resources won't be automatically reclaimed.
            fprintf(stderr, "Error occurred while detaching thread\n");
        }
    }

   // TODO: close `server_fd`
    close(server_fd);
    return 0;
}
