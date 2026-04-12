#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>


// parser
char **parse_input(char *input);
void free_args(char **tokens);
void free_tokens(char **tokens, size_t index);


// executor
int execute_command(char **args, char **env);
int child_process(char **args, char **env);


// helper functions
int string_comp(const char *str1, const char *str2);
int string_ncomp(const char *str1, const char *str2, size_t len);
size_t string_length(const char *str);

char* string_copy(char *dest, const char *src);
char *string_ncopy(char *dest, const char *src, size_t n);
char* string_dup(const char *str);

char **make_env_copy(char **env);
char *getenvironment(const char *name, char **env);

char* string_tok(char *str, const char *delimiter);
char* string_chr(const char *str, int ch);

char* find_command_in_path(char *command, char **env);


// builtin commands
int shell_builtins(char **args, char **env, char *initial_directory);
int command_cd(char **args, char *initial_directory);
int command_pwd();
int command_echo(char **args, char **env);
int command_env(char **env);
int command_which(char **args, char **env);

char **command_setenv(char **args, char **env);
char **command_unsetenv(char **args, char **env);
