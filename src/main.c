#include "shell_resource.h"


void handle_sigint(int sig)
{
    (void) sig;
    printf("\n[shell]> ");
    fflush(stdout);
}

int last_status = 0;

void shell_loop(char **env)
{
    char *input = NULL;
    size_t input_size = 0;
    ssize_t read = 0;
    char **args = NULL;
    char *initial_directory = getcwd(NULL, 0);
    char **env_copy = make_env_copy(env);

    // ignoring signal SIGINT (Ctrl+C) to prevent shell from exiting
    signal(SIGINT, handle_sigint);
    while(1)
    {
        printf("[shell]> ");
        fflush(stdout);
        read = getline(&input, &input_size, stdin);
        
        if(read == -1)
        {
            printf("\n/*Exiting terminal*/\n");
            break;
        }

        if(input[read-1] == '\n')
            input[--read] = '\0';
        
        if(read == 0)
            continue;

        // printf("Input : %s\n", input);
        args = parse_input(input);
        if(args == NULL)
            continue;


        if(args[0] == NULL) {
            free(args);
            continue;

        } else {
            /* this function will extract the execution units (commands seperated by && or ||) and execute them individually
             * address of env_copy is sent for setting and unsetting env variables */
            execute_command_list(args, &env_copy, initial_directory);

        }


        free_args(args);
    }
    free(input);
    free(initial_directory);
}

int main(int argc, char **argv, char **env)
{
    (void) argc;
    (void) argv;
    printf("Entering shell\n");
    shell_loop(env);
    return 0;
}
