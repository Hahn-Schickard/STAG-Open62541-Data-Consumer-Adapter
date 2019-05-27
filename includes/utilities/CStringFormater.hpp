#ifndef C_STRING_FORMATER_UTILITY_HPP
#define C_STRING_FORMATER_UTILITY_HPP

#include <cstring>
#include <cctype>

constexpr unsigned int hash(const char *str, int h = 0)
{
    return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
}

void convertToUpperCase(char *input)
{
    while (*input != '\0')
    {
        if (islower(*input))
            *input = toupper((unsigned char)*input);
            input++;
    }
}

void copyCharArray(const char from[], char to[])
{
    unsigned int stringLenght = strlen(from);
    strncpy(to, from, stringLenght);
    to[stringLenght] = '\0';
}

#endif //C_STRING_FORMATER_UTILITY_HPP