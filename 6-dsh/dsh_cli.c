#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "dshlib.h"
#include "rshlib.h"
#include "dragon.h"

// Enum for operating mode
typedef enum {
    LCLI, // Local CLI
    SCLI, // Server CLI (client mode)
    SSVR  // Server (server mode)
} OpMode;

// Function to parse and process command line arguments
OpMode initParams(int argc, char **argv, char **ip, int *port, int *threaded) {
    int opt;
    OpMode mode = LCLI; // Default mode is local CLI
    *ip = NULL;
    *port = RDSH_DEF_PORT;
    *threaded = 0;
    
    while ((opt = getopt(argc, argv, "csi:p:xh")) != -1) {
        switch (opt) {
            case 'c':
                mode = SCLI; // Client mode
                if (!*ip) *ip = strdup(RDSH_DEF_CLI_CONNECT);
                break;
            case 's':
                mode = SSVR; // Server mode
                if (!*ip) *ip = strdup(RDSH_DEF_SVR_INTFACE);
                break;
            case 'i':
                if (*ip) free(*ip);
                *ip = strdup(optarg);
                break;
            case 'p':
                *port = atoi(optarg);
                break;
            case 'x':
                *threaded = 1; // Enable threaded mode
                break;
            case 'h':
                printf("Usage: %s [-c | -s] [-i IP] [-p PORT] [-x] [-h]\n", argv[0]);
                printf("  Default is to run %s in local mode\n", argv[0]);
                printf("  -c            Run as client\n");
                printf("  -s            Run as server\n");
                printf("  -i IP         Set IP/Interface address (only valid with -c or -s)\n");
                printf("  -p PORT       Set port number (only valid with -c or -s)\n");
                printf("  -x            Enable threaded mode (only valid with -s)\n");
                printf("  -h            Show this help message\n");
                exit(0);
            default:
                fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
                exit(1);
        }
    }
    
    return mode;
}

int main(int argc, char **argv) {
    OpMode mode;
    char *ip_address = NULL;
    int port;
    int threaded;
    int ret = 0;
    
    // Parse command line arguments
    mode = initParams(argc, argv, &ip_address, &port, &threaded);
    
    // Execute based on the mode
    switch (mode) {
        case LCLI:
            // Local mode - execute commands locally
            ret = exec_local_cmd_loop();
            break;
            
        case SCLI:
            // Client mode - connect to server and execute commands remotely
            ret = exec_remote_cmd_loop(ip_address, port);
            break;
            
        case SSVR:
            // Server mode - start server to accept client connections
            ret = start_server(ip_address, port, threaded);
            break;
    }
    
    // Clean up
    if (ip_address) free(ip_address);
    
    return ret;
}
