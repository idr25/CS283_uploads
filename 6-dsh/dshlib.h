#ifndef __DSHLIB_H__
#define __DSHLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

// Constants
#define CMD_ARGV_MAX 64
#define SH_CMD_MAX 1024
#define SH_PROMPT "dsh3> "
#define EXIT_CMD "exit"

// Built-in commands enum
typedef enum {
    BI_NOT_BI,
    BI_CMD_CD,
    BI_CMD_EXIT,
    BI_CMD_DRAGON,
    BI_CMD_STOP_SVR,
    BI_CMD_RC,
    BI_EXECUTED
} Built_In_Cmds;

// Command buffer structure
typedef struct {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;

// Structure to hold multiple piped commands
typedef struct {
    cmd_buff_t commands[CMD_ARGV_MAX];
    int num_cmds;  // Number of commands in the pipeline
} command_list_t;

// Function prototypes
int parse_cmd_line(char *line, cmd_buff_t *cmd);
int handle_cd(cmd_buff_t *cmd);
int execute_external_command(cmd_buff_t *cmd);
int exec_local_cmd_loop(void);
int handle_rc(void);

// New function prototypes for pipes and redirections
int parse_piped_commands(char *line, command_list_t *cmd_list);
int execute_piped_commands(command_list_t *cmd_list);
void handle_redirection(cmd_buff_t *cmd);
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd);

#endif
