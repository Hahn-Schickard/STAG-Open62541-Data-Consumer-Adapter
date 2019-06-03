#ifndef C_STRING_FORMATER_UTILITY_HPP
#define C_STRING_FORMATER_UTILITY_HPP

#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <cctype>

constexpr unsigned int hash(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
}

/*
 * Returns a malloced pointer! 
 * DO NOT FORGET TO USE FREE() ! ! ! 
 */
char *convertToUpperCase(const char *input)
{
    unsigned int stringLenght = strlen(input);
    char *output = (char *)malloc(sizeof(char) * stringLenght);
    for (int i = 0; i < stringLenght; i++)
    {
        if (islower(input[i]))
            output[i] = toupper(input[i]);
        output[i] = input[i];
    }
    return output;
}

void convertToUpperCase(char *input)
{
    unsigned int stringLenght = strlen(input);
    for (int i = 0; i < stringLenght; i++)
    {
        if (islower(input[i]))
            input[i] = toupper(input[i]);
    }
}

void copyCharArray(const char from[], char to[])
{
    unsigned int stringLenght = strlen(from);
    strncpy(to, from, stringLenght);
    to[stringLenght] = '\0';
}

bool identifyBoolean(const char *booleanValue)
{
    char *processedValue = convertToUpperCase(booleanValue);
    bool booleanFlag =
        (strcmp("TRUE", processedValue) || strcmp("Y", processedValue)) ? true
                                                                        : false;
    free(processedValue);
    return booleanFlag;
}

#endif //C_STRING_FORMATER_UTILITY_HPP