#include <stdbool.h>

#ifndef MY_TAR_FLAGS_H
#define MY_TAR_FLAGS_H
typedef struct {
    bool option_f;
    bool option_c;
    char **file_names; //the list of files names
    int n_files; //number of files 
    int n_flags; //number of flags
} my_tar_params; 
#endif


void init_my_tar_params(my_tar_params* tar_params_ptr, int ac, char**av);

void clean_my_tar_params(my_tar_params* tar_params_ptr);

int is_not_flag(char* flag);

int count_flags(int ac, char** av);