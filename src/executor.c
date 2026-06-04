#include "shell_resource.h"

// executes a command using fork and waits for it to complete
int execute_pipelined_command(char **args, char ***env, char *initial_directory)
{
    int status = 0;
    signal(SIGINT, SIG_IGN); // ignoring SIGINT in parent process to prevent it from exiting on Ctrl+C
    /* extract different commands by splitting for pipes | */
    char ***pipelined_commands = split_args(args, "|");
    if(pipelined_commands == NULL) {
        return 1;
    }
    if(pipelined_commands[0] == NULL) { /* empty command */
        free_splitted_args(pipelined_commands);
        return 0;
    }

    /* for all commands execute them and connect pipes */
    int num_commands = 0;
    for(; pipelined_commands[num_commands]; num_commands++);
    int pipe_fds[num_commands-1][2]; /* for n commands we need n-1 pipes */
    for(int i=0; i<num_commands-1; i++) {
        if(pipe(pipe_fds[i]) == -1) {
            perror("bash : pipe");
            for(int j=0; j<i; j++) { /* close already created pipes */
                close(pipe_fds[j][0]);
                close(pipe_fds[j][1]);
            }
            free_splitted_args(pipelined_commands);
            return 1;
        }
    }

    /* fork and execute command */
    pid_t pids[num_commands];
    for(int i=0;i<num_commands;i++) {
        pids[i] = fork();
        if(pids[i] == -1) {
            perror("bash : fork");

            for(int k=0;k<num_commands-1;k++) {
                close(pipe_fds[k][0]);
                close(pipe_fds[k][1]);
            }

            for(int j=0; j<i; j++) {
                kill(pids[j], SIGKILL);
                waitpid(pids[j], NULL, 0);
            }
            free_splitted_args(pipelined_commands);
            return 1;

        /* child process */
        } else if(pids[i] == 0) {
            signal(SIGINT, SIG_DFL); /* restore default SIGINT default behaviour */
            if(i > 0) { /* check if not first command redirect input source */
                dup2(pipe_fds[i-1][0], 0);
                close(pipe_fds[i-1][0]);
            }
            if(i != num_commands -1) { /* check if not last command redirect output source */
                dup2(pipe_fds[i][1], 1);
                close(pipe_fds[i][1]);
            }

            for(int j=0;j<num_commands-1;j++) {
                close(pipe_fds[j][0]);
                close(pipe_fds[j][1]);
            }

            int cmd_status = 0;
            if(is_builtin_command(pipelined_commands[i][0])) {
                cmd_status = shell_builtins(pipelined_commands[i], env, initial_directory);
                exit(cmd_status);
            } else {
                cmd_status = child_process(pipelined_commands[i], *env); /* this will never return */
                exit(cmd_status);
            }
        
        } 
    }

    for(int i = 0; i < num_commands - 1; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }

    for(int i=0; i<num_commands; i++) {
        int wstatus;
        if(waitpid(pids[i], &wstatus, 0) == -1) {
            perror("bash : waitpid");
            return 1;
        }
        int cmd_status = 0;
        if(WIFEXITED(wstatus)) {
            cmd_status = WEXITSTATUS(wstatus);
        } else if(WIFSIGNALED(wstatus)) {
            int sig = WTERMSIG(wstatus);
            if(sig == SIGINT) {
                printf("\n");
            } else {
                fprintf(stderr, "bash : process terminated by signal %d\n", sig);
            }
            cmd_status = 128 + WTERMSIG(wstatus);
        }
        if(cmd_status > 0) { /* store the exit status if returned an exit status code as positive i.e. error occured */
            status = cmd_status;
        }
    }
    free_splitted_args(pipelined_commands);
    return status;
}

// function to execute command in child process
int child_process(char **args, char **env)
{
    // check for redirections and handle them if present
    int saved_stderr = -1;
    int saved_stdout = -1;
    int saved_stdin = -1;
    
    char **filtered_args = apply_redirection(args, &saved_stdin, &saved_stdout, &saved_stderr);
    
    /* error occured during rediretion */
    if(filtered_args == NULL) {
        exit(1);
    } else if(filtered_args[0] == NULL) { /* empty command */
        exit(0);
    }
    
    // find executable in path env
    char *command_path = NULL;

    if(string_chr(filtered_args[0], '/') != NULL) {
        command_path = string_dup(filtered_args[0]);
    } else {
        command_path = find_command_in_path(filtered_args[0], env);
    }


    if (command_path != NULL) { // command found so try to execute
        if(access(command_path, X_OK) == 0) {
            execve(command_path, filtered_args, env);
            perror("bash : execve");
            exit(EXIT_FAILURE);
        } else {
            fprintf(stderr, "%s: permission denied\n", command_path);
            free(command_path);
            for(int i=0; filtered_args[i]; i++)
                free(filtered_args[i]);
            free(filtered_args);
            exit(126);
        }
    }

    fprintf(stderr, "%s: command not found\n", filtered_args[0]);
    free(command_path);
    for(int i=0; filtered_args[i]; i++)
        free(filtered_args[i]);
    free(filtered_args);

    exit(127);
}

