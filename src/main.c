#include "shell_resource.h"

void shell_loop(char **env)
{
    char *input = NULL;
    size_t input_size = 0;
    ssize_t read = 0;
    char **args = NULL;
    char *initial_directory = getcwd(NULL, 0);
    char **env_copy = make_env_copy(env);
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

        // printing arguments as recieved
        // for(size_t i=0;args[i];i++)
        // {
        //     printf("arg %ld: %s\n",i, args[i]);
        // }

        if(args[0] == NULL) {
            free(args);
            continue;

        } else if(!string_comp(args[0], "setenv")) {
            env_copy = command_setenv(args, env_copy); // has potential issues to be fixed later

        } else if(!string_comp(args[0], "unsetenv")) {
            env_copy = command_unsetenv(args, env_copy); // has potential issues to be fixed later

        } else {
            shell_builtins(args, env_copy, initial_directory); // check for builtin commands
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


// repl (done)
// input parsing
// if cmd is std build them as it is to the cmd
// managing path and env variables
// fork and execution
// wait for the child process
// handle the resources to child
// take the resources back from child on completion
// if error occurs handle the error or intrupts 
// promt the user again