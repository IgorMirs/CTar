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
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[8];
    //char version[2]; //???
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char filler[12];
} my_tar_header;

#endif

int init_my_tar_header(my_tar_header* tar_h_ptr, char* fname);

/**
 * @brief   Initialize a string with the a mode of a file
 *
 * @param   n number representing the file mode in octal format
 * @param   k how many digits to consider
 */

void set_name(my_tar_header* tar_head_ptr, char* name, char* total_header);

void set_typeflag(my_tar_header* tar_head_ptr, unsigned mode, char* total_header);

void set_linkname(my_tar_header* tar_head_ptr, char* total_header);

void set_uname(my_tar_header* tar_head_ptr, unsigned uid, char* total_header);

void set_gname(my_tar_header* tar_head_ptr, unsigned uid, char* total_header);

unsigned int count_chksum(char* total_header);

void init_str(unsigned n, char* str, long unsigned size, char* total_header);

void print_tar_header(my_tar_header* tar_head_ptr);

void octal_to_str(unsigned n, char* str, int* i);

void add_leading_zeros(char* str, long unsigned size);

void init_str_wzeros(char* str, unsigned long size);

void init_all_fields_wzeros(my_tar_header* tar_head_ptr);

int strtoi(char* str);

int oct_to_dec(int n);

int my_pow(int base, int pow);
