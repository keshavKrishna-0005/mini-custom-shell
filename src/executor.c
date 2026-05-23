#include "shell_resource.h"

// executes a command using fork and waits for it to complete
int execute_command(char **args, char **env)
{
    pid_t pid;
    int status;
    signal(SIGINT, SIG_IGN); // ignore SIGINT in parent process to prevent shell from exiting on Ctrl+C

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    } else if (pid == 0) { // child process
        // execute the command in the child process
        // restore default behaviour of SIGINT in child process
        signal(SIGINT, SIG_DFL);
        if (child_process(args, env)) {
            exit(EXIT_FAILURE);
        }
    } else { // parent process
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return 1;
        }
        signal(SIGINT, handle_sigint); // restore signal handler for SIGINT in parent process
        if (WIFSIGNALED(status)) {  // check if child process was terminated by a signal
            int sig = WTERMSIG(status);
            if(sig != SIGINT) { // if the signal is not SIGINT, print the signal number
                printf("\nProcess terminated by signal %d\n", sig);
            }
            else {
                printf("\n");
            }
            return 128 + sig; // Standard Unix convention for signal exits
        }
    }
    if (WIFEXITED(status)) { // check if child process was exited normally
        return WEXITSTATUS(status); // Returns 0 for success, 1-255 for errors
    }
    return 1;
}

// function to execute command in child process
int child_process(char **args, char **env)
{
    // check for redirections and handle them if present
    int saved_stderr = -1;
    int buffer_size = 8;
    char **filtered_args = malloc(buffer_size*sizeof(char *));
    int pointer = 0;
    if(filtered_args == NULL) {
        perror("bash : memory allocation failure\n");
        exit(1);
    }

    for(int i=0; args[i]; i++) {

        // input redirection
        if(string_comp(args[i], "<") == 0 && args[i+1] != NULL) {
            int fd_in = open(args[i+1], O_RDONLY);
            if(fd_in < 0) {
                if(saved_stderr != -1) {
                    dup2(saved_stderr, 2);
                }
                fprintf(stderr, "bash : %s: No such file or directory\n", args[i+1]);
                exit(1);
            }
            dup2(fd_in, 0);
            close(fd_in);
            i++;

        // output redirection
        } else if (string_comp(args[i], ">") == 0 && args[i+1] != NULL) {
            int fd_out = open(args[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if(fd_out < 0) {
                if(saved_stderr != -1) {
                    dup2(saved_stderr, 2);
                }
                fprintf(stderr, "bash : error opening %s\n", args[i+1]);
                exit(1);
            }
            dup2(fd_out, 1);
            close(fd_out);
            i++;

        // output append redirection
        } else if (string_comp(args[i], ">>") == 0 && args[i+1] != NULL) {
            int fd_out = open(args[i+1], O_CREAT | O_WRONLY | O_APPEND, 0644);
            if(fd_out < 0) {
                if(saved_stderr != -1) {
                    dup2(saved_stderr, 2);
                }
                fprintf(stderr, "bash : error opening %s\n", args[i+1]);
                exit(1);
            }
            dup2(fd_out, 1);
            close(fd_out);
            i++;

        // error redirection
        } else if (string_comp(args[i], "2>") == 0 && args[i+1] != NULL) {
            if(saved_stderr == -1) saved_stderr = dup(2);

            int fd_err = open(args[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if(fd_err < 0) {
                if(saved_stderr != -1) {
                    dup2(saved_stderr, 2);
                }
                fprintf(stderr, "bash : error opening %s\n", args[i+1]);
                exit(1);
            }
            dup2(fd_err, 2);
            close(fd_err);
            i++;

        // error append redirection
        } else if (string_comp(args[i], "2>>") == 0 && args[i+1] != NULL) {
            if(saved_stderr == -1) saved_stderr = dup(2);

            int fd_err = open(args[i+1], O_CREAT | O_WRONLY | O_APPEND, 0644);
            if(fd_err < 0) {
                if(saved_stderr != -1) {
                    dup2(saved_stderr, 2);
                }
                fprintf(stderr, "bash : error opening %s\n", args[i+1]);
                exit(1);
            }
            dup2(fd_err, 2);
            close(fd_err);
            i++;

        // normal argument
        } else {
            filtered_args[pointer++] = string_dup(args[i]);
            if(pointer >= buffer_size - 1) {
                char **new_ptr = realloc(filtered_args, (buffer_size<<1) * sizeof(char *));
                if(new_ptr == NULL) {
                    if(saved_stderr != -1) {
                        dup2(saved_stderr, 2);
                    }
                    perror("bash : memory allocation failure\n");
                    exit(1);
                }
                filtered_args = new_ptr;
                buffer_size <<= 1;
            }
        }
    }

    filtered_args[pointer] = NULL;

    // safety check
    if(filtered_args[0] == NULL) {
        free(filtered_args); // since no real arguments were added
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

            if(saved_stderr != -1)  dup2(saved_stderr, 2);
            perror("bash : execve");
            exit(EXIT_FAILURE);
        } else {
            if(saved_stderr != -1)  dup2(saved_stderr, 2);

            fprintf(stderr, "%s: permission denied\n", command_path);
            free(command_path);
            for(int i=0; filtered_args[i]; i++)
                free(filtered_args[i]);
            free(filtered_args);
            exit(126);
        }
    }

    if(saved_stderr != -1)  dup2(saved_stderr, 2);

    fprintf(stderr, "%s: command not found\n", filtered_args[0]);
    free(command_path);
    for(int i=0; filtered_args[i]; i++)
        free(filtered_args[i]);
    free(filtered_args);
    exit(127);
}