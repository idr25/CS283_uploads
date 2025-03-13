#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"

/*
 * exec_remote_cmd_loop(server_ip, port)
 *      server_ip:  a string in ip address format, indicating the servers IP
 *                  address.  Note 127.0.0.1 is the default meaning the server
 *                  is running on the same machine as the client
 *              
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -c implemented in dsh_cli.c
 *              For example ./dsh -c 10.50.241.18:5678 where 5678 is the new port
 *              number and the server address is 10.50.241.18    
 */
int exec_remote_cmd_loop(char *address, int port)
{
    char *cmd_buff;
    char *rsp_buff;
    int cli_socket;
    ssize_t io_size;
    int is_eof;

    // Allocate buffers for command and response
    cmd_buff = malloc(RDSH_COMM_BUFF_SZ);
    rsp_buff = malloc(RDSH_COMM_BUFF_SZ);
    
    if (cmd_buff == NULL || rsp_buff == NULL) {
        return client_cleanup(-1, cmd_buff, rsp_buff, ERR_MEMORY);
    }

    // Connect to server
    cli_socket = start_client(address, port);
    if (cli_socket < 0) {
        perror("start client");
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }

    // Main command loop
    while (1) {
        // Print prompt and get input from user
        printf(SH_PROMPT);
        fflush(stdout);
        
        if (!fgets(cmd_buff, RDSH_COMM_BUFF_SZ, stdin)) {
            break; // EOF from stdin
        }
        
        // Remove newline if present
        size_t len = strlen(cmd_buff);
        if (len > 0 && cmd_buff[len-1] == '\n') {
            cmd_buff[len-1] = '\0';
            len--;
        }
        
        // Skip empty commands
        if (len == 0) {
            continue;
        }
        
        // Send command to server (including null terminator)
        io_size = send(cli_socket, cmd_buff, len + 1, 0);
        if (io_size <= 0) {
            perror("Error sending command to server");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }
        
        // Receive response from server
        while (1) {
            io_size = recv(cli_socket, rsp_buff, RDSH_COMM_BUFF_SZ, 0);
            
            if (io_size < 0) {
                perror("Error receiving data from server");
                return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
            }
            
            if (io_size == 0) {
                // Server closed connection
                printf(RCMD_SERVER_EXITED);
                return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
            }
            
            // Check if this is the last chunk (ends with EOF char)
            is_eof = (rsp_buff[io_size - 1] == RDSH_EOF_CHAR) ? 1 : 0;
            
            // Print response (excluding EOF char if present)
            if (is_eof) {
                printf("%.*s", (int)(io_size - 1), rsp_buff);
            } else {
                printf("%.*s", (int)io_size, rsp_buff);
            }
            
            // If this was the last chunk, break the receive loop
            if (is_eof) {
                break;
            }
        }
        
        // Check if we need to exit (if last command was "exit" or "stop-server")
        if (strcmp(cmd_buff, "exit") == 0 || strcmp(cmd_buff, "stop-server") == 0) {
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
        }
    }

    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}

/*
 * start_client(server_ip, port)
 *      server_ip:  a string in ip address format, indicating the servers IP address.
 *      port:   The port the server will use.
 */
int start_client(char *server_ip, int port) {
    struct sockaddr_in addr;
    int cli_socket;
    int ret;

    // Create socket
    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    // Set up server address
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    // Convert IP address string to network format
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    // Connect to server
    ret = connect(cli_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("connect");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    return cli_socket;
}

/*
 * client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
 *      cli_socket:   The client socket
 *      cmd_buff:     The buffer that will hold commands to send to server
 *      rsp_buff:     The buffer that will hold server responses
 */
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc){
    //If a valid socket number close it.
    if(cli_socket > 0){
        close(cli_socket);
    }

    //Free up the buffers 
    free(cmd_buff);
    free(rsp_buff);

    //Echo the return value that was passed as a parameter
    return rc;
}
