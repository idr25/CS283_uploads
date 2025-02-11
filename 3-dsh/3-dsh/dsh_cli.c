#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dshlib.h"

int main()
{
    char cmd_buff[SH_CMD_MAX]; // Buffer for user input
    command_list_t clist;      // Command list structure

    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        // Remove trailing newline
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Check for "exit" command
        if (strcmp(cmd_buff, EXIT_CMD) == 0)
        {
            exit(0);
        }

        // **Handle the "dragon" command separately**
        if (strcmp(cmd_buff, "dragon") == 0)
        {
            print_dragon_compressed();  // Call function from dshlib.c
            printf("\n[DRAGON for extra credit!]\n");
            continue; // Skip normal parsing
        }

        // Parse command input
        int result = build_cmd_list(cmd_buff, &clist);
        if (result == WARN_NO_CMDS)
        {
            printf(CMD_WARN_NO_CMD);
        }
        else if (result == ERR_TOO_MANY_COMMANDS)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        }
        else if (result == OK)
        {
            printf(CMD_OK_HEADER, clist.num);
            for (int i = 0; i < clist.num; i++)
            {
                printf("<%d> %s", i + 1, clist.commands[i].exe);
                if (strlen(clist.commands[i].args) > 0)
                {
                    printf(" [%s]", clist.commands[i].args);
                }
                printf("\n");
            }
        }
    }

    return 0;
}

