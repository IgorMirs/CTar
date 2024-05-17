#include "my_string.h"
#include <unistd.h>

int my_strcmp(const char* s1, const char* s2) {
    int i = 0;
    while(s1[i] != 0 && s2[i] != 0) {
        if (s1[i] > s2[i]) {
            return 1;
        }
        else if (s1[i] < s2[i]) {
            return -1;
        }
        i++;
    }
    //strings are equal
    if (s1[i] == 0 && s2[i] == 0) {
        return 0; 
    }
    //s1 is longer than s2
    else if (s1[i] != 0) {
        return 1;
    }
    //s2 is longer than s1
    else {
        return -1;
    }
}

int my_putchar(char c) {
  return write(1, &c, 1);
}

void my_puts(const char* s) {
    while (*s != 0) {
        my_putchar(*s);
        s++;
    }
    my_putchar('\n');
}

int my_strlen(const char* s) {
    int size = 0;
    while (*s != 0) {
        size++;
        s++;
    }
    return size;
}

char* my_strcpy(char* s1, char* s2) {
    if (s1 == NULL) {
        return NULL;
    }
    int i = 0, j = 0;
    while(s2[j] != 0) {
        s1[i] = s2[j++];
        i++;
    }
    s1[i] = '\0';
    return s1;
}

char* my_strcat(char* s1, char* s2) {
    int i = 0;
    while(s1[i] != 0) {
        i++;
    }
    int j = 0;
    while (s2[j] != 0) {
        s1[i++] = s2[j];
        j++;
    }

    s1[i] = '\0';

    return s1;
}