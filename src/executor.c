#include "shell_resource.h"

// executes a command using fork and waits for it to complete
int execute_command(char **args, char **env)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    } else if (pid == 0) { // child process
        if (child_process(args, env)) {
            exit(EXIT_FAILURE);
        }
    } else { // parent process
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return 1;
        }
        if (WIFSIGNALED(status)) {
            printf("Process terminated by signal %d\n", WTERMSIG(status));
        }
    }
    return 0;
}

// function to execute command in child process
int child_process(char **args, char **env)
{
    char *command_path = find_command_in_path(args[0], env);
    if (command_path != NULL) { // command found so try to execute
        if(access(command_path, X_OK) == 0) {
            execve(command_path, args, env);
            perror("execve");
            exit(EXIT_FAILURE);
        } else {
            fprintf(stderr, "%s: permission denied\n", command_path);
            exit(EXIT_FAILURE);
        }
    }
    free(command_path);


    // check current directory for executables
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    size_t len = (string_length(args[0]) + string_length(cwd) + 2);
    char *full_path = (char *) malloc(sizeof(char) * len);
    if(full_path == NULL) {
        perror("malloc");
        free(cwd);
        exit(EXIT_FAILURE);
    }
    snprintf(full_path, len, "%s/%s", cwd, args[0]);

    if (access(full_path, X_OK) == 0) {
        execve(full_path, args, env);
        perror("execve");
        
    } else {
        fprintf(stderr, "%s: command not found\n", args[0]);
    }
    // no such command present, print error message
    free(full_path);
    free(cwd);
    exit(EXIT_FAILURE);
}