#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>

// Constants
#define CMD_ARGV_MAX 64
#define SH_CMD_MAX 1024
#define SH_PROMPT "dsh2> "
#define EXIT_CMD "exit"

// Command buffer structure
typedef struct cmd_buff {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;

// Global status variable
extern int last_status;

// Function prototypes
int parse_cmd_line(char *line, cmd_buff_t *cmd);
int handle_cd(cmd_buff_t *cmd);
int execute_external_command(cmd_buff_t *cmd);
void exec_local_cmd_loop(void);
int handle_rc(void);

#endif

