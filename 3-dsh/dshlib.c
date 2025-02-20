#include "dshlib.h"
#include <errno.h>

int last_status = 0;

void trim_whitespace(char *str) {
    if (!str) return;
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = 0;
}

int parse_cmd_line(char *line, cmd_buff_t *cmd) {
    if (!line || !cmd) return -1;

    char *clean_line = strdup(line);
    trim_whitespace(clean_line);

    if (strlen(clean_line) == 0) {
        cmd->argc = 0;
        free(clean_line);
        return 0;
    }

    cmd->_cmd_buffer = clean_line;
    cmd->argc = 0;
    int in_quote = 0;
    char quote_char = 0;
    char *current = clean_line;
    char *start = clean_line;

    while (*current) {
        if (in_quote) {
            if (*current == quote_char) {
                in_quote = 0;
                *current = '\0';
                cmd->argv[cmd->argc++] = start;
                start = current + 1;
            }
        } else {
            switch (*current) {
            case ' ':
            case '\t':
                *current = '\0';
                if (start != current) {
                    cmd->argv[cmd->argc++] = start;
                }
                while (*(current + 1) == ' ' || *(current + 1) == '\t') current++;
                start = current + 1;
                break;
            case '\'':
            case '"':
                in_quote = 1;
                quote_char = *current;
                start = current + 1;
                break;
            default:
                break;
            }
        }
        current++;
    }

    if (start != current) {
        cmd->argv[cmd->argc++] = start;
    }

    cmd->argv[cmd->argc] = NULL;
    return 0;
}

int handle_cd(cmd_buff_t *cmd) {
    if (cmd->argc == 1) {
        const char *home = getenv("HOME");
        if (home) {
            if (chdir(home) != 0) {
                perror("dsh: cd failed");
                last_status = errno;
                return -1;
            }
        }
        return 0;
    }

    if (cmd->argc > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        last_status = 1;
        return -1;
    }

    if (chdir(cmd->argv[1]) != 0) {
        fprintf(stderr, "cd: %s\n", strerror(errno));
        last_status = errno;
        return -1;
    }

    last_status = 0;
    return 0;
}

int execute_external_command(cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("dsh: fork failed");
        last_status = errno;
        return -1;
    }

    if (pid == 0) { // Child
        execvp(cmd->argv[0], cmd->argv);
        
        // Handle specific execvp() errors
        switch (errno) {
            case ENOENT:
                fprintf(stderr, "dsh: command not found: %s\n", cmd->argv[0]);
                break;
            case EACCES:
                fprintf(stderr, "dsh: permission denied: %s\n", cmd->argv[0]);
                break;
            default:
                fprintf(stderr, "dsh: exec error: %s\n", strerror(errno));
        }
        exit(errno);
    } else { // Parent
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            last_status = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            last_status = WTERMSIG(status);
        } else {
            last_status = -1;
        }
    }
    return 0;
}

int handle_rc(void) {
    printf("%d\n", last_status);
    return 0;
}

void exec_local_cmd_loop(void) {
    char input[SH_CMD_MAX];
    while (1) {
        printf(SH_PROMPT);
        fflush(stdout);

        if (!fgets(input, SH_CMD_MAX, stdin)) {
            if (feof(stdin)) {
                printf("\n");
                break;
            } else {
                perror("fgets");
                break;
            }
        }

        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;

        cmd_buff_t cmd;
        if (parse_cmd_line(input, &cmd) == 0) {
            if (cmd.argc > 0) {
                if (strcmp(cmd.argv[0], "exit") == 0) {
                    free(cmd._cmd_buffer);
                    exit(0);
                } else if (strcmp(cmd.argv[0], "cd") == 0) {
                    handle_cd(&cmd);
                } else if (strcmp(cmd.argv[0], "rc") == 0) {
                    handle_rc();
                } else {
                    execute_external_command(&cmd);
                }
                free(cmd._cmd_buffer);
            }
        }
    }
}

