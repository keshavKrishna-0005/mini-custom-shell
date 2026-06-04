#include "shell_resource.h"

char** parse_input(char *input)
{
    size_t buffer_size = 8;
    char **tokens = malloc(buffer_size*sizeof(char *));
    char *token = NULL;
    size_t token_length = 0;
    
    if(!tokens)
    {
        perror("bash : memory allocation failure");
        exit(1);
    }
    size_t position = 0;
    for(size_t i=0;input[i];)
    {
        token_length = 0;
        while(input[i] && (input[i] == ' ' || input[i] == '\t' || input[i] == '\r' || input[i] =='\a'))
            i++;
        
        if(input[i] == '"')
        {
            token = &input[++i];
            while(input[i] != '"')
            {
                if(!input[i])
                {
                    perror("Invalid input"); // should also free memory
                    free_tokens(tokens, position);
                    return NULL;
                }
                token_length++;
                i++;
            }
            i++;
        }
        else
        {
            token = &input[i];
            while(input[i] && (input[i] != ' ' && input[i] != '\t'))
            {
                token_length++;
                i++;
            }
        }

        if(token_length != 0)
        {
            tokens[position] = malloc((token_length+1)*sizeof(char));
            if(!tokens[position])
            {
                perror("bash : memory allocation failure"); // should also free space
                free_tokens(tokens, position);
                exit(1);
            }

            for(size_t j=0;j<token_length;j++)
            {
                tokens[position][j] = token[j];
            }
            tokens[position][token_length] = '\0'; 
            position++;
            if(position == buffer_size)
            {
                char **ptr = realloc(tokens, (buffer_size<<1)*sizeof(char *));
                if(ptr == NULL)
                {
                    perror("bash : memory allocation failure"); // should also free space
                    exit(1);
                }
                tokens = ptr;
                buffer_size <<=1;
            }
        }

    }

    tokens[position] = NULL;
    return tokens;
}


// free all allocated memories because error occured while making tokens
void free_tokens(char **tokens, size_t index)
{
    if(!tokens)
        return;
    for(int i=index-1; i>=0; i--)
    {
        free(tokens[i]);
    }
    free(tokens);
}


// free all the allocated memories not needed
void free_args(char **tokens)
{
    if(!tokens)
    {
        return;
    }
    for(size_t i = 0; tokens[i]; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}


/* splits args array into 2D array based on the token provided */
char ***split_args(char **args, const char *token)
{
    int buffer_size = 2; /* default buffere_size */
    char ***splitted_args = (char ***) malloc((buffer_size) * sizeof(char *)); /* (char *) because all pointers are of same size */
    if(splitted_args == NULL) {
        perror("bash : memory allocation failure");
        exit(1);
    }
    int num_commands = 0;
    int arg_count = 0;
    int group_start = 0;
    int i=0;
    for(; args[i]; i++) {

        if(string_comp(args[i], token) == 0) { /* split here */

            if(arg_count == 0) {
                perror("bash : invalid command format");
                splitted_args[num_commands] = NULL;
                free_splitted_args(splitted_args);
                return NULL;
            }


            splitted_args[num_commands] = (char **) malloc((arg_count+1) * sizeof(char *)); /* +1 to terminate each of them with NULL */
            if(splitted_args[num_commands] == NULL) {
                perror("bash : memory allocation failure");
                splitted_args[num_commands] = NULL;
                free_splitted_args(splitted_args);
                exit(1);
            }
            for(int j=0;j<arg_count;j++) {
                splitted_args[num_commands][j] = string_dup(args[group_start+j]);
            }


            splitted_args[num_commands++][arg_count] = NULL;
            if(num_commands == buffer_size) {
                char *** new_ptr = realloc(splitted_args, (buffer_size<<1)*sizeof(char *));
                if(new_ptr == NULL) {
                    perror("bash : memory allocation failure");
                    exit(1);
                }
                splitted_args = new_ptr;
                buffer_size <<=1;
            }
            arg_count = 0;
            group_start = i + 1;
        } else {
            arg_count++;
        }
    }
    /* final command*/
    if(arg_count != 0) {
        splitted_args[num_commands] = (char **) malloc((arg_count+1) * sizeof(char *)); /* +1 to terminate each of them with NULL */
        if(splitted_args[num_commands] == NULL) {
            perror("bash : memory allocation failure\n");
            free_splitted_args(splitted_args);
            exit(1);
        }

        for(int j=0;j<arg_count;j++) {
            splitted_args[num_commands][j] = string_dup(args[group_start+j]);
        }
        splitted_args[num_commands++][arg_count] = NULL;
        
    } else if(arg_count == 0 && num_commands > 0) { /* no command after split token */
        perror("bash : invalid command format");
        splitted_args[num_commands] = NULL;
        free_splitted_args(splitted_args);
        return NULL;
    }

    /* terminate the splited_commands array with NULL */
    if(num_commands == buffer_size) {
        char *** new_ptr = realloc(splitted_args, (buffer_size+1)*sizeof(char *));
        if(new_ptr == NULL) {
            perror("bash : memory allocation failure");
            exit(1);
        }
        splitted_args = new_ptr;
        buffer_size += 1;
    }
    splitted_args[num_commands] = NULL;

    return splitted_args;
}

void free_splitted_args(char ***splitted_args)
{
    if(splitted_args == NULL)
        return;
    for(int i=0; splitted_args[i]; i++) {
        for(int j=0; splitted_args[i][j]; j++) {
            free(splitted_args[i][j]);
        }
        free(splitted_args[i]);
    }
    free(splitted_args);
}

