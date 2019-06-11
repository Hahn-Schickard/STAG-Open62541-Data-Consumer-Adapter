#include "CStringFormater.hpp"

constexpr unsigned int hash(const char *str, int h) {
  return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
}

char *getMallocedUpperCase(const char *input) {
  unsigned int string_lenght = strlen(input);
  char *output = (char *)malloc(sizeof(char) * string_lenght);
  for (int i = 0; i < string_lenght; i++) {
    if (islower(input[i]))
      output[i] = toupper(input[i]);
    output[i] = input[i];
  }
  return output;
}

void convertToUpperCase(char *input) {
  unsigned int string_lenght = strlen(input);
  for (int i = 0; i < string_lenght; i++) {
    if (islower(input[i]))
      input[i] = toupper(input[i]);
  }
}

void copyCharArray(const char from[], char to[]) {
  unsigned int string_lenght = strlen(from);
  strncpy(to, from, string_lenght);
  to[string_lenght] = '\0';
}

bool identifyBoolean(const char *boolean_value) {
  char *processed_value = getMallocedUpperCase(boolean_value);
  bool boolean_flag =
      (strcmp("TRUE", processed_value) || strcmp("Y", processed_value)) ? true
                                                                        : false;
  free(processed_value);
  return boolean_flag;
}
