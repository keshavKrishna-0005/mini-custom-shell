#include "shell_resource.h"


// permission issues,cd -  (not implemented)
int command_cd(char **args, char *initial_directory)
{
    (void) initial_directory;
    char *path = args[1];
    if(path == NULL) { // reach home directory 
        path = getenv("HOME");
    }

    char *home_dir = NULL;
    if((home_dir = string_chr(path, '~')) != NULL) { // check if directory name starts with ~
        if(home_dir == path) { // check if ~user or ~/ or actual path

            if((path[1] != 0) && (path[1] == '/')) { // cd ~/actual_path
                char *home_env = getenv("HOME");
                if(home_env == NULL) {
                    printf("cd: HOME environment variable not set\n");
                    return 1;
                }
                size_t new_path_len = string_length(home_env) + string_length(path);
                char *new_path = (char *)malloc((new_path_len) * sizeof(char));
                if(new_path == NULL) {
                    perror("malloc failure");
                    return 1;
                }
                snprintf(new_path, new_path_len, "%s%s", home_env, path + 1);
                path = new_path;
            }

            else if(path[1] != 0) { // cd ~user
                printf("cd: user directories not supported\n");
                return 1;
            } else { // cd ~
                char *home_env = getenv("HOME");
                if(home_env == NULL) {
                    printf("cd: HOME environment variable not set\n");
                    return 1;
                }
                path = home_env;
            }
        }
    }
    if(chdir(path) != 0) {
        perror("cd");
    }
    return 0;
}

// done
int command_pwd()
{
    char *cwd = NULL;
    if ((cwd = getcwd(NULL, 0)) == NULL){
        perror("getcwd");
    } else {
        printf("%s\n", cwd);
        free(cwd);
    }
    return 0;
}

// done
int command_echo(char **args, char **env)
{
    int new_line = 1; // default echo with new line
    size_t i = 1; // argument position we start with

    if (args[i] != NULL && string_comp(args[i], "-n") == 0) {
        new_line = 0;
        i++; // skipping -n for printing
    }

    for(; args[i]; i++) {
        
        if (args[i][0] == '$') { // handle env variables
            char *value = getenvironment(args[i] + 1, env); // skipping $ from $PATH
            if (value) {
                printf("%s", value);
            } 
        } else {
            printf("%s", args[i]);
        }

        if (args[i+1] != NULL) {
            printf(" ");
        }

    }

    if (new_line) {
        printf("\n");
    }

    return 0;
}

// done
int command_env(char **env)
{
    for(size_t i = 0; env[i]; i++)
    {
        printf("%s\n", env[i]);
    }
    return 0;
}


// done
int command_which(char **args, char **env)
{
    (void) env;
    if(args[1] == NULL) {
        printf("which: expected arguments\n");
        return 1;
    }

    // list of built in commands
    const char* built_in_commands[] = {"cd", "pwd", "echo", "env", "unsetenv", "setenv", "which", "exit", "quit", NULL};
    for(size_t i=0; built_in_commands[i]; i++)
    {
        if(string_comp(args[1], built_in_commands[i]) == 0)
        {
            printf("%s: shell built-in command\n", args[1]);
            return 0;
        }
    }

    // check external commands
    char* full_path = find_command_in_path(args[1], env);
    if(full_path != NULL) {
        printf("%s\n", full_path);
        free(full_path);
        return 0;
    } else {
        printf("which : %s command not found\n", args[1]);
        return 1;
    }
}

// Function to set environment variables
char **command_setenv(char **args, char **env)
{
    if(args[1] == NULL) {
        printf("Usage: setenv <variable>=<val>\nor: setenv <variable> <value>\n");
        return env;
    }

    
    char *var = NULL;
    char *val = NULL;
    // first check the format of the input, if its setenv VAR=VAL or setenv VAR VAL
    if(string_chr(args[1], '=') == NULL) { // format VAR val
        if(args[2] == NULL) {
            printf("Usage: setenv <variable>=<val>\nor: setenv <variable> <value>\n");
            return env;
        }
        var = string_dup(args[1]);
        val = string_dup(args[2]);
    } else { // format VAR=VAL
        char *equal_sign = string_chr(args[1], '=');
        val = string_dup(equal_sign + 1);
        size_t len = equal_sign-args[1];
        var = (char *)malloc((len+1) * sizeof(char));
        if(!var) {
            perror("malloc failure");
            free(val);
            return env;
        }
        for (size_t i=0; i < len; i++) {
            var[i] = args[1][i];
        }
        var[len] = '\0';
    }

    // validate variable name
    if(var == NULL || val == NULL) {
        printf("Memory allocation failed\n");
        free(var);
        free(val);
        return env;
    }


    // count the variables
    size_t env_count = 0;
    for(size_t i=0; env[i]; i++) {
        env_count++;
        size_t len = string_length(var);
        // check if variable already exists, if it does we will update it and return
        if ((string_ncomp(var, env[i], len) == 0) && (env[i][len] == '=')) {
            char *new_entry = (char *)malloc((string_length(var) + string_length(val) + 2)*sizeof(char));
            if(!new_entry) {
                perror("malloc failure");
                free(var);
                free(val);
                return env;
            }
            snprintf(new_entry, string_length(var) + string_length(val) + 2, "%s=%s", var, val);
            free(env[i]);
            env[i] = new_entry;
            free(var);
            free(val);
            return env;
        }

    }

    // make a duplicate env variable
    char **new_env = (char**) malloc((env_count + 2) *sizeof(char*));
    if(!new_env) {
        perror("malloc failure");
        return env;
        }
    for(size_t j=0; env[j]; j++) {
        new_env[j] = string_dup(env[j]);
    }
    char *new_entry = (char *)malloc((string_length(var) + string_length(val) + 2)*sizeof(char));
    if(!new_entry) {
        perror("malloc failure");
        for(size_t i=0; i<env_count; i++) {
            free(new_env[i]);
        }
        free(new_env);
        return env;
    }
    snprintf(new_entry, string_length(var) + string_length(val) + 2, "%s=%s", var, val);
    
    new_env[env_count] = new_entry;
    free(var);
    free(val);
    new_env[env_count+1] = NULL;
    // free the old env variable
    for(size_t i=0; i<env_count; i++) {
        free(env[i]);
    }
    free(env);
    return new_env;
}

// Function to unset environment variables
char **command_unsetenv(char **args, char **env)
{
    if(args[1] == NULL) {
        printf("Usage: unsetenv <variable>\n");
        return env;
    }

    size_t env_count = 0;
    size_t position = 0;
    int found = 0;
    for(int i=0; env[i]; i++){
        env_count++;
        if((string_ncomp(args[1], env[i], string_length(args[1])) == 0) && (env[i][string_length(args[1])] == '=')){
            position = i;
            found = 1;
        }
    }

    if(!found) {
        printf("unsetenv: variable not found\n");
        return env;
    }
    

    char **new_env = (char **)malloc((env_count) * sizeof(char *));
    if(!new_env) {
        perror("malloc failure");
        return env;
    }

    size_t i=0;
    for(size_t index = 0; index<env_count; index++) {
        if(index == position) {
            free(env[index]);
            continue;
        }
        new_env[i] = string_dup(env[index]);
        free(env[index]);
        i++;
    }
    new_env[i] = NULL;
    free(env);
    return new_env;
}


// cd, pwd, echo, env, setenv, unsetenv, which, exit
int shell_builtins(char **args, char **env, char *initial_directory)
{
    if(!string_comp(args[0], "cd")){
        return command_cd(args, initial_directory);

    } else if(!string_comp(args[0], "pwd")){
        return command_pwd();

    } else if(!string_comp(args[0], "echo")){
        return command_echo(args, env);

    } else if(!string_comp(args[0], "env")){
        return command_env(env);

    } else if(!string_comp(args[0], "which")){
        return command_which(args, env);

    } else if(!string_comp(args[0], "exit") || !string_comp(args[0], "quit")){
        exit(EXIT_SUCCESS);

    } else{
        // Not a builtin command, execute it as an external command
        return execute_command(args, env);
    }
    return 0;
}

