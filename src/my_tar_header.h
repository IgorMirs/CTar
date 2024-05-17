#ifdef __APPLE__
#ifndef st_mtim
#define st_mtim st_mtimespec
#endif
#endif

#ifndef MY_TAR_HEADER_H
#include "my_tar_params.h"
#include <stdbool.h>

#define MY_TAR_HEADER_H
typedef struct {
    char name[100];
    char mode[8];
    char uid[8]; 
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8]; //TODO
    char typeflag; //TODO
    char linkname[100];
    char magic[6];  //TODO
    char version[2]; //???
    char uname[32];
    char gname[32];
    bool isValidHeader;
    char total_header[500];
} my_tar_header;

#endif

void init_my_tar_header(my_tar_header* tar_head_ptr, my_tar_params* tar_param_ptr); //add my_tar_flag here

/**
 * @brief   Initialize a string with the a mode of a file
 * 
 * @param   n number representing the file mode in octal format
 * @param   k how many digits to consider 
 */

void set_name(my_tar_header* tar_head_ptr, char* name);

void set_typeflag(my_tar_header* tar_head_ptr, unsigned mode);

void set_linkname(my_tar_header* tar_head_ptr);

void set_uname(my_tar_header* tar_head_ptr, unsigned uid);

void set_gname(my_tar_header* tar_head_ptr, unsigned uid);

unsigned int count_chksum(my_tar_header* tar_head_ptr);

void init_str(my_tar_header* tar_head_ptr, unsigned n, char* str, long unsigned size);

void print_tar_header(my_tar_header* tar_head_ptr);

void octal_to_str(unsigned n, char* str, int* i);

void add_leading_zeros(char* str, long unsigned size);