#include "shell_resource.h"

char** parse_input(char *input)
{
    size_t buffer_size = 8;
    char **tokens = malloc(buffer_size*sizeof(char *));
    char *token = NULL;
    size_t token_length = 0;
    
    if(!tokens)
    {
        perror("Malloc failure\n");
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
                    perror("Invalid input\n"); // should also free memory
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
                perror("Malloc failure\n"); // should also free space
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
                    perror("Realloc failure\n"); // should also free space
                    free_tokens(tokens, position);
                    return NULL;
                }
                printf("Entered here\n");
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


