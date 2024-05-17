#include <stdbool.h>

#ifndef MY_TAR_FLAGS_H
#define MY_TAR_FLAGS_H
typedef struct {
    bool option_f;
    bool option_c;
    bool option_x;
    bool option_t;
    bool option_r;
    bool option_u;
    bool valid_flag;
    char* archive_name; //the name of an archieve
    char **file_names; //the list of files names
    int n_files; //number of files
    int n_flags; //number of flags


} my_tar_params;
#endif


int init_my_tar_params(my_tar_params* tar_params_ptr, int ac, char**av);

void init_with_default(my_tar_params* tar_p_ptr);

void clean_my_tar_params(my_tar_params* tar_params_ptr);

int is_not_flag(char* flag);

int count_flags(my_tar_params* tar_params_ptr, int ac, char** av);
