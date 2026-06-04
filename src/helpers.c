#include "shell_resource.h"

// function to search for command in path env
char* find_command_in_path(char *command, char **env)
{
    char *path_env = getenvironment("PATH", env);
    if (path_env == NULL) 
        return NULL;
    char *path = string_dup(path_env); // duplicate of path env to work with
    if(path == NULL) {
        perror("string_dup");
        return NULL;
    }


    size_t command_len = string_length(command);
    char *full_path = NULL;
    char *token = string_tok(path, ":");

    while(token != NULL) {
        size_t token_len = string_length(token);
        size_t path_len = (token_len + command_len + 2);
        full_path = (char *)malloc((path_len)*sizeof(char));

        if(token_len > 0 && token[token_len - 1] == '/'){
            snprintf(full_path, path_len, "%s%s", token, command); // construct a full_path
        } else {
            snprintf(full_path, path_len, "%s%s%s", token, "/", command); // construct a full_path
        }

        if(access(full_path, X_OK) == 0) { // checking if command exists here as executable
            free(path);
            return full_path;
        }

        token = string_tok(NULL, ":");
        free(full_path);
    }

    free(path);
    return NULL;   
}


// makes a copy of env variables to work with, so that we can modify it without affecting the original env
char **make_env_copy(char **env)
{
    size_t env_count = 0;
    for(size_t i=0; env[i]; i++) {
        env_count++;
    }

    char **env_copy = (char **)malloc((env_count + 1) * sizeof(char *));
    if(!env_copy) {
        perror("bash : memory allocation failure");
        return NULL;
    }
    for(size_t i=0; i<env_count; i++) {
        env_copy[i] = string_dup(env[i]);
        if(!env_copy[i]) {
            perror("string_dup failure");
            for(size_t j=0; j<i; j++) {
                free(env_copy[j]);
            }
            free(env_copy);
            return NULL;
        }
    }
    env_copy[env_count] = NULL;
    return env_copy;
}


// get specific environment variable's value
char *getenvironment(const char *name, char **env)
{
    if(name == NULL || env == NULL)
        return NULL;

    size_t name_len = string_length(name);
    for(int i = 0; env[i]; i++)
    {
        if((string_ncomp(env[i], name, name_len) == 0) && (env[i][name_len] == '='))
        {
            return &env[i][name_len + 1];
        }
    }
    return NULL;
}


// check if function is a built in command
int is_builtin_command(const char *command)
{
    for(size_t i=0; built_in_commands[i]; i++) {
        if(string_comp(built_in_commands[i], command) == 0) {
            return 1;
        }
    }
    return 0;
}


/* does the redirection and filtere_args is returned
 * if some error occurs during redirection then NULL is returned 
 * original args are not freed */
char **apply_redirection(char **args, int *saved_stdin, int *saved_stdout, int *saved_stderr)
{
    char *error_filename = NULL;
    int error_during_redirection = -1; /* 0 - error opening input redirection, 1 - error redirecting output/error, 2 - malloc/realloc error */
    
    int buffer_size = 8; // default buffer size
    char **filtered_args = malloc(sizeof(char *)*buffer_size);
    if(filtered_args == NULL) {
        perror("bash : memory allocation failure");
        return NULL;
    }

    int pointer = 0;
    for(size_t i=0; args[i]; i++) {

        /* input redirection */
        if(string_comp(args[i], "<") == 0 && args[i+1] != NULL) { // i+1 contains filename (input redirected)
            if(*saved_stdin == -1) *saved_stdin = dup(0); // saving stdin

            int fd_in = open(args[i+1], O_RDONLY);
            if(fd_in < 0) {
                error_during_redirection = 0; 
                error_filename = string_dup(args[i+1]); 
                break;
            }

            dup2(fd_in, 0);
            close(fd_in);
            i++;

        // output trucate redirection
        } else if(string_comp(args[i], ">") == 0 && args[i+1] != NULL) {
            if(*saved_stdout == -1) *saved_stdout = dup(1); // saving stdout

            int fd_out = open(args[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if(fd_out < 0) {
                error_during_redirection = 1; 
                error_filename = string_dup(args[i+1]); 
                break;
            }

            dup2(fd_out, 1);
            close(fd_out);
            i++;

        // output append redirection
        } else if(string_comp(args[i], ">>") == 0 && args[i+1] != NULL) {
            if(*saved_stdout == -1) *saved_stdout = dup(1); // saving stdout

            int fd_out = open(args[i+1], O_CREAT | O_WRONLY | O_APPEND, 0644);
            if(fd_out < 0) {
                error_during_redirection = 1; 
                error_filename = string_dup(args[i+1]);
                break;
            }

            dup2(fd_out, 1);
            close(fd_out);
            i++;

        // error truncate redirection
        } else if(string_comp(args[i], "2>") == 0 && args[i+1] != NULL) {
            if(*saved_stderr == -1) *saved_stderr = dup(2); // saving stderr

            int fd_out = open(args[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if(fd_out < 0) {
                error_during_redirection = 1; 
                error_filename = string_dup(args[i+1]);
                break;
            }

            dup2(fd_out, 2);
            close(fd_out);
            i++;

        // error append redirection
        } else if(string_comp(args[i], "2>>") == 0 && args[i+1] != NULL) {
            if(*saved_stderr == -1) *saved_stderr = dup(2); // saving stderr

            int fd_out = open(args[i+1], O_CREAT | O_WRONLY | O_APPEND, 0644);
            if(fd_out < 0) {
                error_during_redirection = 1; 
                error_filename = string_dup(args[i+1]); 
                break;
            }

            dup2(fd_out, 2);
            close(fd_out);
            i++;

        // normal argument
        } else { 
            filtered_args[pointer++] = string_dup(args[i]); // filtering args
            if(pointer >= buffer_size-1) {
                char **new_ptr = realloc(filtered_args, (buffer_size<<1)*sizeof(char *));
                if(new_ptr == NULL) {
                    error_during_redirection = 2; // realloc failure
                    break;
                }
                filtered_args = new_ptr;
                buffer_size <<= 1;
            }
        }
    }
    if(error_during_redirection == -1) {
        filtered_args[pointer] = NULL;
        return filtered_args;
    }

    /* error occured during redirection*/
    redirection_restore(*saved_stdin, *saved_stdout, *saved_stderr);
    if(filtered_args != NULL) {
        for (int i=0;i<pointer;i++) {
            if(filtered_args[i] != NULL)
                free(filtered_args[i]);
        }
        free(filtered_args);
    }
    if(error_during_redirection == 0) {
        fprintf(stderr, "bash : %s: No such file or directory\n", error_filename);
        free(error_filename);

    } else if(error_during_redirection == 1) {
        fprintf(stderr, "bash : error opening %s\n", error_filename);
        free(error_filename);

    } else if(error_during_redirection == 2) {
        fprintf(stderr, "bash : memory allocation failure\n");
        /* no need to free error_filename here */
    }
    return NULL;
}


/* restoring the redirections created within a command if needed */
void redirection_restore(int saved_stdin, int saved_stdout, int saved_stderr)
{
    if(saved_stdin != -1) {
        dup2(saved_stdin, 0);
        close(saved_stdin);
    }
    if(saved_stdout != -1) {
        dup2(saved_stdout, 1);
        close(saved_stdout);
    }
    if(saved_stderr != -1) {
        dup2(saved_stderr, 2);
        close(saved_stderr);
    }
}

