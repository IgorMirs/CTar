#ifndef STRING_H
#define STRING_H

/**
 * @brief   Compares two given strings.
 * @return  1 if s1 > s2
 *          0 if s1 == s2
 *          -1 if s1 < s2
 */
int my_strcmp(const char* s1, const char* s2);

/**
 * @brief   Prints the given character
 * 
 * @param   c character
 * @return  int ???
 */
int my_putchar(char c);

/**
 * @brief   Prints the given string
 * 
 * @param   s string
 */
void my_puts(const char* s);

/**
 * @brief   Counts the length (excluding the null character) 
 *          of the given string
 * 
 * @param   s string 
 * @return  int length 
 */
int my_strlen(const char* s);

/**
 * @brief   Copies the content of s2 to s1 
 * 
 * @param   s1 the first string 
 * @param   s2 the second string 
 * @return  char* the pointer to the first string 
 */
char* my_strcpy(char* s1, char* s2);

/**
 * @brief   Concatenates (joins) two strings.
 * 
 * @param   s1 the first string 
 * @param   s2 the second string 
 * @return  char* the pointer to the first string 
 */
char* my_strcat(char* s1, char* s2);

#endif