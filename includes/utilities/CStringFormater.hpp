#ifndef C_STRING_FORMATER_UTILITY_HPP_
#define C_STRING_FORMATER_UTILITY_HPP_

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <stdio.h>

constexpr unsigned int hash(const char *str, int h = 0);

/*
 * Returns a malloced pointer!
 * DO NOT FORGET TO USE FREE() ! ! !
 */
char *getMallocedUpperCase(const char *input);

void convertToUpperCase(char *input);

void copyCharArray(const char from[], char to[]);

bool identifyBoolean(const char *boolean_value);

#endif // C_STRING_FORMATER_UTILITY_HPP_