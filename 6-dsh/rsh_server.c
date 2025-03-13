#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

// INCLUDES for extra credit
#include <signal.h>
#include <pthread.h>
// -------------------------

#include "dshlib.h"
#include "rshlib.h"
#include "dragon.h"

// Global variables for threading
int is_threaded_mode = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * set_threaded_server(val) - For extra credit
 * Sets whether the server should operate in threaded mode
 */
void set_threaded_server(int val) {
    is_threaded_mode = val;
}

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.
 *      port:   The port the server will use.
 *      is_threaded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 */
int start_server(char *ifaces, int port, int is_threaded) {
    int svr_socket;
    int rc;

    // Set threaded mode if requested (extra credit)
    set_threaded_server(is_threaded);

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        int err_code = svr_socket;  // server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server() function.
 */
int stop_server(int svr_socket) {
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  
 */
int boot_server(char *ifaces, int port) {
    int svr_socket;
    int ret;
    
    struct sockaddr_in addr;

    // Create socket
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket == -1) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    // Enable address reuse
    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    // Prepare the address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // Convert IP address string to network format
    if (inet_pton(AF_INET, ifaces, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    // Bind socket to address
    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */
    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * Thread function for handling clients
 * Used for the extra credit part
 */
void *handle_client(void *arg) {
    int cli_socket = *((int *)arg);
    free(arg);  // Free the memory allocated for the socket pointer
    
    // Execute client requests in the thread
    exec_client_requests(cli_socket);
    
    // Close the client socket when done
    close(cli_socket);
    
    return NULL;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 */
int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc = OK;
    struct sockaddr_in cli_addr;
    socklen_t addr_len = sizeof(cli_addr);
    
    while (1) {
        // Accept a client connection
        cli_socket = accept(svr_socket, (struct sockaddr *)&cli_addr, &addr_len);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }
        
        if (is_threaded_mode) {
            // Extra credit: Handle client in a separate thread
            pthread_t thread;
            int *client_sock = malloc(sizeof(int));
            if (!client_sock) {
                perror("malloc");
                close(cli_socket);
                continue;
            }
            *client_sock = cli_socket;
            
            if (pthread_create(&thread, NULL, handle_client, client_sock) != 0) {
                perror("pthread_create");
                free(client_sock);
                close(cli_socket);
                continue;
            }
            
            // Detach thread so it cleans up automatically when done
            pthread_detach(thread);
        } else {
            // Non-threaded mode: Handle client sequentially
            rc = exec_client_requests(cli_socket);
            
            // Close client socket
            close(cli_socket);
            
            // If client requested to stop server, break out of loop
            if (rc == OK_EXIT) {
                printf(RCMD_MSG_SVR_STOP_REQ);
                break;
            }
        }
    }
    
    return rc;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 */
int exec_client_requests(int cli_socket) {
    int io_size;
    command_list_t cmd_list;
    int cmd_rc;
    char *io_buff;
    
    // Allocate buffer for communication
    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL) {
        return ERR_RDSH_SERVER;
    }
    
    memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
    
    while (1) {
        // Receive command from client
        ssize_t total_bytes = 0;
        int found_null = 0;
        
        // Loop until we find a null byte (end of command marker)
        while (!found_null && total_bytes < RDSH_COMM_BUFF_SZ - 1) {
            io_size = recv(cli_socket, io_buff + total_bytes, 
                           RDSH_COMM_BUFF_SZ - total_bytes - 1, 0);
            
            if (io_size <= 0) {
                // Error or client closed connection
                if (io_size < 0) {
                    perror("recv");
                }
                free(io_buff);
                return OK;  // Client disconnected
            }
            
            // Check for null byte in received data
            for (int i = 0; i < io_size; i++) {
                if (io_buff[total_bytes + i] == '\0') {
                    found_null = 1;
                    break;
                }
            }
            
            total_bytes += io_size;
        }
        
        // Ensure null termination
        io_buff[total_bytes] = '\0';
        
        // Check for built-in commands first
        cmd_buff_t cmd;
        memset(&cmd, 0, sizeof(cmd));
        if (parse_cmd_line(io_buff, &cmd) != 0) {
            send_message_string(cli_socket, "Error parsing command\n");
            send_message_eof(cli_socket);
            continue;
        }
        
        // Handle built-in commands
        if (strcmp(cmd.argv[0], "exit") == 0) {
            free(io_buff);
            printf(RCMD_MSG_CLIENT_EXITED);
            return OK;
        } else if (strcmp(cmd.argv[0], "stop-server") == 0) {
            free(io_buff);
            return OK_EXIT;
        } else if (strcmp(cmd.argv[0], "cd") == 0) {
            // Handle cd command
            if (cmd.argc < 2) {
                const char *home = getenv("HOME");
                if (home) {
                    chdir(home);
                }
            } else {
                if (chdir(cmd.argv[1]) != 0) {
                    char error_msg[256];
                    snprintf(error_msg, sizeof(error_msg), "cd: %s: %s\n", 
                             cmd.argv[1], strerror(errno));
                    send_message_string(cli_socket, error_msg);
                }
            }
            send_message_eof(cli_socket);
            continue;
        } else if (strcmp(cmd.argv[0], "dragon") == 0) {
            // Handle dragon command (extra credit)
            // Redirect stdout temporarily to capture dragon output
            int stdout_save = dup(STDOUT_FILENO);
            int pipe_fds[2];
            pipe(pipe_fds);
            dup2(pipe_fds[1], STDOUT_FILENO);
            close(pipe_fds[1]);
            
            // Print dragon
            print_dragon_compressed();
            fflush(stdout);
            
            // Restore stdout
            dup2(stdout_save, STDOUT_FILENO);
            close(stdout_save);
            
            // Read the output from the pipe
            char dragon_buffer[RDSH_COMM_BUFF_SZ];
            ssize_t bytes_read = read(pipe_fds[0], dragon_buffer, RDSH_COMM_BUFF_SZ - 1);
            close(pipe_fds[0]);
            
            if (bytes_read > 0) {
                dragon_buffer[bytes_read] = '\0';
                send_message_string(cli_socket, dragon_buffer);
            }
            
            send_message_eof(cli_socket);
            continue;
        }
        
        // Parse command into a pipeline if it contains pipes
        memset(&cmd_list, 0, sizeof(cmd_list));
        if (strchr(io_buff, '|')) {
            // Command contains pipes
            if (parse_piped_commands(io_buff, &cmd_list) != 0 || cmd_list.num_cmds == 0) {
                send_message_string(cli_socket, "Error parsing piped command\n");
                send_message_eof(cli_socket);
                continue;
            }
        } else {
            // Single command
            cmd_list.num_cmds = 1;
            cmd_list.commands[0] = cmd;
        }
        
        // Execute the command pipeline
        cmd_rc = rsh_execute_pipeline(cli_socket, &cmd_list);
        
        // Send EOF to signal end of command output
        send_message_eof(cli_socket);
    }
    
    free(io_buff);
    return OK;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 */
int send_message_eof(int cli_socket) {
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 */
int send_message_string(int cli_socket, char *buff) {
    if (!buff) return ERR_RDSH_COMMUNICATION;
    
    size_t len = strlen(buff);
    ssize_t sent_len = send(cli_socket, buff, len, 0);
    
    if ((size_t)sent_len != len) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), CMD_ERR_RDSH_SEND, (int)sent_len, (int)len);
        send(cli_socket, error_msg, strlen(error_msg), 0);
        return ERR_RDSH_COMMUNICATION;
    }
    
    return OK;
}

/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure to execute
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int num_commands = clist->num_cmds;
    int pipes[2 * (num_commands - 1)];
    pid_t pids[num_commands];
    int status = 0;
    
    // Create pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes + i * 2) < 0) {
            perror("pipe");
            return -1;
        }
    }
    
    // Fork and execute each command
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork");
            return -1;
        }
        
        if (pids[i] == 0) {  // Child process
            // Redirect input if not the first command
            if (i > 0) {
                dup2(pipes[(i - 1) * 2], STDIN_FILENO);
            } else {
                // First command gets input from socket
                dup2(cli_sock, STDIN_FILENO);
            }
            
            // Redirect output if not the last command
            if (i < num_commands - 1) {
                dup2(pipes[i * 2 + 1], STDOUT_FILENO);
            } else {
                // Last command sends output to socket
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            }
            
            // Close all pipe ends in child process
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipes[j]);
            }
            
            // Execute the command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            
            // If execvp fails
            fprintf(stderr, "execvp failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    // Close all pipes in parent process
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipes[i]);
    }
    
    // Wait for all child processes
    for (int i = 0; i < num_commands; i++) {
        int child_status;
        waitpid(pids[i], &child_status, 0);
        if (WIFEXITED(child_status)) {
            status = WEXITSTATUS(child_status);
        }
    }
    
    return status;
}

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 */
Built_In_Cmds rsh_match_command(const char *input) {
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype) {
    case BI_CMD_DRAGON:
        print_dragon_compressed();
        return BI_EXECUTED;
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        if (cmd->argc > 1) {
            chdir(cmd->argv[1]);
        } else {
            const char *home = getenv("HOME");
            if (home) {
                chdir(home);
            }
        }
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}
