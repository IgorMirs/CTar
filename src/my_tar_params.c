#include "my_string.h"
#include "my_tar_params.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int init_my_tar_params(my_tar_params* tar_params_ptr, int ac, char**av) {
    init_with_default(tar_params_ptr);
    tar_params_ptr->n_flags = count_flags(tar_params_ptr, ac, av);
    int n_provided_files = 0;

    //no flags (just ./my_tar) or invalid flags
    if (tar_params_ptr->n_flags == -1 || tar_params_ptr->valid_flag == false) {
        char err_msg[] = "my_tar: Must specify one of -c, -r, -t, -u, -x\n";
        write(2, err_msg, sizeof(err_msg) / sizeof(char));
        return 1; //error
    }
    //no flags (just ./my_tar [filenames...])
    else if (tar_params_ptr->n_flags == 0) {
        char err_msg[] = "Usage:\n"
            "  List:    my_tar -tf <archive-filename>\n"
            "  Extract: my_tar -xf <archive-filename>\n"
            "  Create:  my_tar -cf <archive-filename> [filenames...]";
        write(2, err_msg, sizeof(err_msg) / sizeof(char));
        return 1; //error
    }
    //no f option
    else if(tar_params_ptr->option_f == false) {
        char err_msg[] = "my_tar: Refusing to write archive contents to terminal (missing -f option?)";
        write(2, err_msg, sizeof(err_msg) / sizeof(char));
        return 1; //error
    }
    //only flags with no files
    else if ((n_provided_files = ac - tar_params_ptr->n_flags - 1) == 0) {
        char err_msg[] = "my_tar: no files or directories specified";
        write(2, err_msg, sizeof(err_msg) / sizeof(char));
        return 1; //error
    }
    //try to create without archive or filenames
    else if ((tar_params_ptr->option_c == true || tar_params_ptr->option_r == true || tar_params_ptr->option_u == true)
             && n_provided_files == 1) {
        char err_msg[] = "Usage:\n"
            "  Create:  my_tar -cf <archive-filename> [filenames...]\n"
            "  Update:  my_tar -uf <archive-filename> [filenames...]\n"
            "  Append:  my_tar -rf <archive-filename> [filenames...]";
        write(2, err_msg, sizeof(err_msg) / sizeof(char));
        return 1; //error
    }
    //valid parameters (at least two files)
    else {
        //set the archive name
        int av_index = tar_params_ptr->n_flags + 1;
        tar_params_ptr->archive_name = (char*) malloc((my_strlen(av[av_index]) + 1) * sizeof(char));
        my_strcpy(tar_params_ptr->archive_name, av[av_index]);
        av_index++;
        n_provided_files--;

        //set filenames
        tar_params_ptr->file_names = (char**) malloc(sizeof(char*) * (n_provided_files));
        int names_index = 0;
        //insert files names in the array of strings
        struct stat* buf = (struct stat*) malloc(sizeof(struct stat));

        while (names_index < n_provided_files) {
            DIR *dr;

            //if the provided file/dir name is invalid
            if (lstat(av[av_index], buf) != 0 && (dr = opendir(av[av_index])) == NULL) {
                char err_msg1[] = "my_tar: ";
                char err_msg2[] = " Cannot stat: No such file or directory\n";
                write(2, err_msg1, sizeof(err_msg1) / sizeof(char));
                write(2, av[av_index], my_strlen(av[av_index]));
                write(2, err_msg2, my_strlen(err_msg2));
                free(buf);
                return 1; //error
            }

            tar_params_ptr->file_names[names_index] = (char*) malloc((my_strlen(av[av_index]) + 2) * sizeof(char));
            my_strcpy(tar_params_ptr->file_names[names_index], av[av_index]);
            tar_params_ptr->n_files++;
            //if the name is a directory
            if ((dr = opendir(av[av_index])) != NULL) {
                //add / to the end of a name
                my_strcat(tar_params_ptr->file_names[names_index], "/");
                closedir(dr);
            }
            names_index++;
            av_index++;
        } //end while
        free(buf);
    } //end else

    return 0; //no error
}

void clean_my_tar_params(my_tar_params* tar_params_ptr) {
    if (tar_params_ptr->file_names != NULL) {
        for(int i = 0; i < tar_params_ptr->n_files; i++) {
            free(tar_params_ptr->file_names[i]);
        }
        free(tar_params_ptr->file_names);

        free(tar_params_ptr->archive_name);
    }
}

int is_not_flag(char* flag) {
    return(my_strlen(flag) <= 1 || flag[0] != '-');
}

int count_flags(my_tar_params* tar_params_ptr, int ac, char** av) {
    int n_flags = 0;
    //no parameters
    if (ac <= 1) {
        return -1;
    }
    for (int i = 1; i < ac; i++) {
        if(!is_not_flag(av[i])) {
            n_flags++;
            //check flag options
            for(int j = 1; j < my_strlen(av[i]); j++) {
                if (av[i][j] == 'c') {
                    tar_params_ptr->option_c = true;
                }
                else if(av[i][j] == 'f') {
                    tar_params_ptr->option_f = true;
                }
                else if(av[i][j] == 't') {
                    tar_params_ptr->option_t = true;
                }
                else if(av[i][j] == 'x') {
                    tar_params_ptr->option_x = true;
                }
                else if(av[i][j] == 'r') {
                    tar_params_ptr->option_r = true;
                }
                else if(av[i][j] == 'u') {
                    tar_params_ptr->option_u = true;
                }
                else {
                    tar_params_ptr->valid_flag = false;
                    break;
                }
            }
        }
        else {
            break;
        }
    }

    return n_flags;
}


void init_with_default(my_tar_params* tar_params_ptr) {
    tar_params_ptr->file_names = NULL;
    tar_params_ptr->archive_name = NULL;
    tar_params_ptr->n_files = 0;
    tar_params_ptr->option_c = false;
    tar_params_ptr->option_f = false;
    tar_params_ptr->option_t = false;
    tar_params_ptr->option_x = false;
    tar_params_ptr->option_r = false;
    tar_params_ptr->option_u = false;
    tar_params_ptr->valid_flag = true;
}
