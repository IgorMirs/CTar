#include "my_string.h"
#include "my_tar_params.h"
#include <stdlib.h>
#include <stdbool.h>

void init_my_tar_params(my_tar_params* tar_params_ptr, int ac, char**av) {
    tar_params_ptr->file_names = NULL;
    tar_params_ptr->n_files = 0;
    tar_params_ptr->n_flags = count_flags(ac, av);

    //no flags (just ./my_tar)
    if (tar_params_ptr->n_flags == -1) {
        my_puts("my_tar: Must specify one of -c, -r, -t, -u, -x");
    }
    //invalid parameters
    else if (tar_params_ptr->n_flags == 0) {
        my_puts("Usage:\n"
            "  List:    my_tar -tf <archive-filename>\n"
            "  Extract: my_tar -xf <archive-filename>\n"
            "  Create:  my_tar -cf <archive-filename> [filenames...]");
    }
    else if ((tar_params_ptr->n_files = ac - tar_params_ptr->n_flags - 1) == 0) {
        my_puts("my_tar: no files or directories specified");
    }
    //valid parameters
    else {
        tar_params_ptr->file_names = (char**) malloc(sizeof(char*) * (tar_params_ptr->n_files));
        
        int av_index = tar_params_ptr->n_flags + 1;
        int names_index = 0;
        //insert files names in the array of strings
        while (names_index < tar_params_ptr->n_files) {
            tar_params_ptr->file_names[names_index] = (char*) malloc((my_strlen(av[av_index]) + 1) * sizeof(char));
            my_strcpy(tar_params_ptr->file_names[names_index], av[av_index]);
            names_index++;
            av_index++;
        }
    }
}

void clean_my_tar_params(my_tar_params* tar_params_ptr) {
    for(int i = 0; i < tar_params_ptr->n_files; i++) {
        free(tar_params_ptr->file_names[i]);
    }
    if (tar_params_ptr->file_names != NULL) {
        free(tar_params_ptr->file_names);
    }
}

int is_not_flag(char* flag) {
    return(my_strlen(flag) <= 1 || flag[0] != '-');
}

int count_flags(int ac, char** av) {
    int res = 0;
    if (ac <= 1) {
        return -1;
    }
    for (int i = 1; i < ac; i++) {
        if(!is_not_flag(av[i])) {
            res++;
        }
        else {
            break;
        }
    }

    return res;
}

