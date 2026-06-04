#include "shell_resource.h"

void execute_command_list(char **args, char ***env, char *initial_directory)
{
    /* first iterate over args to separate commands over layer 1 i.e. && or || or ; */
    int group_start = 0;
    int arg_count = 0;
    
    int continue_execution = 1; /* for handling && and || */
    int op = -1; /* 0 for &&, 1 for ||, -1 for ; or no operator */
    for(size_t i=0; args[i] && continue_execution; i++) {
        if(string_comp(args[i], "&&") == 0 || string_comp(args[i], "||") == 0 || string_comp(args[i], ";") == 0) {
            if(arg_count == 0) {
                fprintf(stderr, "bash : invalid command format\n");
                return;
            }
            if(string_comp(args[i], "&&") == 0) {
                op = 0;
            } else if(string_comp(args[i], "||") == 0) {
                op = 1;
            } else {
                op = -1;
            }

            char **command_args = (char **)malloc((arg_count + 1) * sizeof(char *));
            if(!command_args) {
                perror("bash : memory allocation failure");
                exit(1);
            }
            for(int j=0; j<arg_count; j++) {
                command_args[j] = string_dup(args[group_start + j]);
            }
            command_args[arg_count] = NULL;
            int status = execute_command(command_args, env, initial_directory);
            if(op == 0 && status != 0) { /* && operator and command failed */
                continue_execution = 0;
            } else if(op == 1 && status == 0) { /* || operator and command succeeded */
                continue_execution = 0;
            }
            last_status = status;
            for(int j=0; j<arg_count; j++) {
                free(command_args[j]);
            }
            free(command_args);
            group_start = i + 1;
            arg_count = 0;
        
        } else {
            arg_count++;
        }
    }
    if(arg_count > 0 && continue_execution) {
        char **command_args = (char **)malloc((arg_count + 1) * sizeof(char *));
        if(!command_args) { 
            perror("bash : memory allocation failure"); 
            exit(1); 
        }
        for(int j = 0; j < arg_count; j++) {
            command_args[j] = string_dup(args[group_start + j]);
        }
        command_args[arg_count] = NULL;
        last_status = execute_command(command_args, env, initial_directory);
        for(int j = 0; j < arg_count; j++) free(command_args[j]);
        free(command_args);
    }
}

int execute_command(char **args, char ***env, char *initial_directory)
{
    /* check if only single command present builtin type then no fork needed */
    /* else pass it to execute_pipelined_command (needs changes) */
    if(args[0] == NULL) {
        return 0;
    }
    for(size_t i=0; args[i]; i++) {
        if(string_comp(args[i], "|") == 0) {
            return execute_pipelined_command(args, env, initial_directory);
        }
    }

    if(is_builtin_command(args[0])) {
        return shell_builtins(args, env, initial_directory);
    }

    return execute_pipelined_command(args, env, initial_directory);
}

