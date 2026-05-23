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
        perror("bash : memory allocation failure\n");
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



