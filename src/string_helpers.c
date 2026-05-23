#include "shell_resource.h"

// > 0 : if str1 > str2
// 0 : if str1 == str2
// < 0 : if str1 < str2
int string_comp(const char *str1, const char *str2)
{
    while(*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return (unsigned char)(*str1) - (unsigned char)(*str2);
}

// > 0 : if str1 > str2
// 0 : if str1 == str2
// < 0 : if str1 < str2
int string_ncomp(const char *str1, const char *str2, size_t len)
{
    size_t i=0;
    while(i < len && str1[i] && str2[i]) {
        if(str1[i] != str2[i]) {
            return (unsigned char)(str1[i]) - (unsigned char)(str2[i]);
        }
        i++;
    }

    if (i == len)
        return 0;
    
    return (unsigned char)(str1[i]) - (unsigned char)(str2[i]);
    
}

// returns length of string, for null returns 0
size_t string_length(const char *str)
{   
    size_t len = 0;

    if(str == NULL)
        return len;
    
    while((*str))
    {
        len++;
        str++;
    }
    return len;
}

// copies src string to dest and returns dest, if src is null returns null
char* string_copy(char *dest, const char *src)
{
    if(src == NULL)  return NULL;
    char* ret = dest;
    while(*src){
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return ret;
}

// copies src string to dest and returns pointer to the duplicate (only upto n characters), if src is null returns null
char *string_ncopy(char *dest, const char *src, size_t n)
{
    if(src == NULL) return NULL;
    char *ret = dest;
    size_t i=0;
    while(i < n && *src) {
        *dest = *src;
        dest++;
        src++;
        i++;
    }
    while(i < n) {
        *dest = '\0';
        i++;
        dest++;
    }
    *dest = '\0';
    return ret;
}

// duplicates a string and returns pointer to the duplicate, if str is null returns null
char* string_dup(const char *str)
{
    if(str == NULL)
        return NULL;
    
    size_t len = string_length(str);
    char *buf = (char *)malloc((len + 1) * sizeof(char));

    if(buf == NULL) return NULL;

    string_copy(buf, str);
    return buf;
}

// tokenizes a string based on the given delimiters and returns pointer to the token, 
// subsequent calls with str as null will return next tokens, 
// returns null if no more tokens are available
char* string_tok(char *str, const char *delimiter)
{
    static char *input = NULL;

    if(str != NULL) {
        input = str;
    }

    if(input == NULL){
        return NULL;
    }

    while(*input && string_chr(delimiter, *input)) {
        input++;
    }
    char *start=input;

    if(*input == '\0') {
        input = NULL;
        return NULL;
    }

    char *end = start;
    while(*end && !string_chr(delimiter, *end)) {
        end++;
    }

    if(*end == '\0') {
        input = NULL;
    } else {
        input = end+1;
        *end = '\0';
    }

    return start;
}

// returns add of first char matching in str
char* string_chr(const char *str, int ch)
{
    while(*str) {
        if(*str == ch) {
            return (char *)str;
        }
        str++;
    }
    return NULL;
}
