#include "my_tar_header.h"
#include "my_string.h"
#include "my_tar_params.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pwd.h>
#include <grp.h>

///*****************************/*

bool isOnBit(int num, int pos) {
	int mask = 1;
	mask = mask << pos;
	return num & mask;
}

void displayBits(int num) {
	for (int i = sizeof(num) * 8 - 1; i >= 0; i--) {
		my_putchar(isOnBit(num, i) ? '1' : '0');
	}
	putchar('\n');
}

///*****************************/*





int init_my_tar_header(my_tar_header* tar_head_ptr, char* fname) {
    //init all fields with '\0'
    init_all_fields_wzeros(tar_head_ptr);

    char plain_header[512];
    init_str_wzeros(plain_header, sizeof(plain_header) / sizeof(char));

    struct stat* buf = (struct stat*) malloc(sizeof(struct stat));
    //invalid name of a file or directory
    if (lstat(fname, buf) != 0) {
        char err_msg1[] = "my_tar: ";
        char err_msg2[] = " Cannot stat: No such file or directory\n";
        write(2, err_msg1, sizeof(err_msg1) / sizeof(char));
        write(2, fname, my_strlen(fname));
        write(2, err_msg2, sizeof(err_msg2) / sizeof(char));

        free(buf);
        return 1; //error
    }

    //set name
    set_name(tar_head_ptr, fname, plain_header);
    //set mode
    //% 512 (778 in octal) to consider only three last digits in mode
    init_str(buf->st_mode % 01000, tar_head_ptr->mode, sizeof(tar_head_ptr->mode) / sizeof(char), plain_header);
    //set uid
    init_str(buf->st_uid, tar_head_ptr->uid, sizeof(tar_head_ptr->uid)/ sizeof(char), plain_header);
    //set gid
    init_str(buf->st_gid, tar_head_ptr->gid, sizeof(tar_head_ptr->gid)/ sizeof(char), plain_header);
    //set size
    if (S_ISDIR(buf->st_mode)) { //for directory the size is 0
        init_str(0, tar_head_ptr->size, sizeof(tar_head_ptr->size)/ sizeof(char), plain_header);
    }
    else {
        init_str(buf->st_size, tar_head_ptr->size, sizeof(tar_head_ptr->size)/ sizeof(char), plain_header);
    }
    //set mtime
    init_str(buf->st_mtim.tv_sec, tar_head_ptr->mtime, sizeof(tar_head_ptr->mtime)/ sizeof(char), plain_header);


    //set typeflag
    set_typeflag(tar_head_ptr, buf->st_mode, plain_header);

    //set linkname
    set_linkname(tar_head_ptr, plain_header);

    //set magic
    my_strcpy(tar_head_ptr->magic, "ustar  ");
    my_strcat(plain_header, tar_head_ptr->magic);

    //set version
    //my_strcpy(tar_head_ptr->version, "  ");
    //my_strcat(total_header, tar_head_ptr->version);


    //set uname
    set_uname(tar_head_ptr, buf->st_uid, plain_header);

    //set gname
    set_gname(tar_head_ptr, buf->st_gid, plain_header);

    //set checksum
    init_str(count_chksum(plain_header), tar_head_ptr->chksum, sizeof(tar_head_ptr->chksum) / sizeof(char) - 1, plain_header);
    int last_position = my_strlen(tar_head_ptr->chksum);
    tar_head_ptr->chksum[last_position + 1] = ' ';

    //devmajor  devminor prefix and filler - empty for now

    free(buf);

    return 0; // success
}


void set_name(my_tar_header* tar_head_ptr, char* name, char* total_header) {
    //set all the characters to '\0'
    init_str_wzeros(tar_head_ptr->name, sizeof(tar_head_ptr->name) / sizeof(char));
    my_strcpy(tar_head_ptr->name, name);

    //add to total
    my_strcpy(total_header, name);
}

void set_typeflag(my_tar_header* tar_head_ptr, unsigned mode, char* total_header) {
    //regular file
    if (S_ISREG(mode)) {
        tar_head_ptr->typeflag = '0';
    }
    else if (S_ISLNK(mode)){
        tar_head_ptr->typeflag = '2';
    }
    else if(S_ISDIR(mode)) {
        tar_head_ptr->typeflag = '5';
    }

    //add to total
    char temp[2];
    temp[0] = tar_head_ptr->typeflag;
    temp[1] = '\0';
    my_strcat(total_header, temp);
}

void set_linkname(my_tar_header* tar_head_ptr, char* total_header) {
    int bufsize = sizeof(tar_head_ptr->linkname) / sizeof(char) - 1;
    char* buf = (char*) malloc(bufsize);
    int nbytes = readlink(tar_head_ptr->name, buf, bufsize);
    if (nbytes != -1) {
        buf[nbytes] = '\0';
        my_strcpy(tar_head_ptr->linkname, buf);
    }
    //the file is not a link
    else {
        tar_head_ptr->linkname[0] = '\0';
    }

    //add to total
    my_strcat(total_header, tar_head_ptr->linkname);

    free(buf);
}

void set_uname(my_tar_header* tar_head_ptr, unsigned uid, char* total_header) {
    struct passwd *pwd;

    pwd = getpwuid(uid);
    if (pwd != NULL) {
        my_strcpy(tar_head_ptr->uname, pwd->pw_name);
    }
    else {
        tar_head_ptr->uname[0] = '\0';
    }

    //add to total
    my_strcat(total_header, tar_head_ptr->uname);
}

void set_gname(my_tar_header* tar_head_ptr, unsigned gid, char* total_header) {
    struct group *grp;

    grp = getgrgid(gid);
    if (grp != NULL) {
        my_strcpy(tar_head_ptr->gname, grp->gr_name);
    }
    else {
        tar_head_ptr->gname[0] = '\0';
    }

    //add to total
    my_strcat(total_header, tar_head_ptr->gname);
}

unsigned int count_chksum(char* total_header) {
    int sum = 0;
    for(int i = 0; i < my_strlen(total_header); i++) {
        sum += total_header[i];
    }

    sum += 256;

    return sum;
}

void init_str(unsigned n, char* str, long unsigned size, char* total_header) {
    int i = 0;
    if (n == 0) { //the parameter is 0
        str[0] = '\0';
    }
    else {
        octal_to_str(n, str, &i);
    }
    add_leading_zeros(str, size);

    //add to total
    my_strcat(total_header, str);
}

void print_tar_header(my_tar_header* tar_head_ptr) {
    if (isZeroString(tar_head_ptr->size, sizeof(tar_head_ptr->size) / sizeof(char))) {
        return;
    }
    printf("%s", tar_head_ptr->name);
    printf("%s", tar_head_ptr->mode);
    printf("%s", tar_head_ptr->uid);
    printf("%s", tar_head_ptr->gid);
    printf("%s", tar_head_ptr->size);
    printf("%s", tar_head_ptr->mtime);
    printf("%s", tar_head_ptr->chksum);
    printf("%c", tar_head_ptr->typeflag);
    printf("%s", tar_head_ptr->linkname);
    printf("%s", tar_head_ptr->magic);
    //printf("%s", tar_head_ptr->version);
    printf("%s", tar_head_ptr->uname);
    printf("%s", tar_head_ptr->gname);

}

void octal_to_str(unsigned n, char* str, int* i) {
    //if the given number is 0
    if (n == 0 && *i == 0) {
        str[*i] = '\0';
        return;
    }
    else if (n == 0) {
        return;
    }
    else {
        octal_to_str(n / 8, str, i);
        str[*i] = n % 8 + '0';
        (*i)++;
    }
    //set the terminating null character
    str[*i] = '\0';
}

void add_leading_zeros(char* str, long unsigned size) {
    int n_zeros = size - my_strlen(str) - 1;
    if (n_zeros <= 0) {
        return;
    }
    //creating the temp string and adding the required number of zeros
    char* temp_str = (char*) malloc(sizeof(char) * size);
    int i = 0;
    for(; i < n_zeros; i++) {
        temp_str[i] = '0';
    }
    temp_str[i] = '\0';
    my_strcat(temp_str, str);
    my_strcpy(str, temp_str);

    free(temp_str);
}

void init_str_wzeros(char* str, unsigned long size) {
    for(unsigned long i = 0; i < size; i++) {
        str[i] = '\0';
    }
}

void init_all_fields_wzeros(my_tar_header* tar_head_ptr) {
    /*
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
    */

    init_str_wzeros(tar_head_ptr->name, sizeof(tar_head_ptr->name) / sizeof(char));
    init_str_wzeros(tar_head_ptr->mode, sizeof(tar_head_ptr->mode) / sizeof(char));
    init_str_wzeros(tar_head_ptr->uid, sizeof(tar_head_ptr->uid) / sizeof(char));
    init_str_wzeros(tar_head_ptr->gid, sizeof(tar_head_ptr->gid) / sizeof(char));
    init_str_wzeros(tar_head_ptr->size, sizeof(tar_head_ptr->size) / sizeof(char));
    init_str_wzeros(tar_head_ptr->mtime, sizeof(tar_head_ptr->mtime) / sizeof(char));
    init_str_wzeros(tar_head_ptr->chksum, sizeof(tar_head_ptr->chksum) / sizeof(char));
    tar_head_ptr->typeflag = '\0';
    init_str_wzeros(tar_head_ptr->linkname, sizeof(tar_head_ptr->linkname) / sizeof(char));
    init_str_wzeros(tar_head_ptr->magic, sizeof(tar_head_ptr->magic) / sizeof(char));
    init_str_wzeros(tar_head_ptr->uname, sizeof(tar_head_ptr->uname) / sizeof(char));
    init_str_wzeros(tar_head_ptr->gname, sizeof(tar_head_ptr->gname) / sizeof(char));
    init_str_wzeros(tar_head_ptr->devmajor, sizeof(tar_head_ptr->devmajor) / sizeof(char));
    init_str_wzeros(tar_head_ptr->devminor, sizeof(tar_head_ptr->devminor) / sizeof(char));
    init_str_wzeros(tar_head_ptr->prefix, sizeof(tar_head_ptr->prefix) / sizeof(char));
    init_str_wzeros(tar_head_ptr->filler, sizeof(tar_head_ptr->filler) / sizeof(char));
}


int strtoi(char* str) {
    int res = 0;
    bool lead_zero = true;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '0' && lead_zero) {
            continue;
        }
        else if (str[i] != '0' && lead_zero) {
            lead_zero = false;
        }

        if (!lead_zero) {
            res *= 10;
            res += str[i] - '0';
            lead_zero = false;
        }
    }

    return res;
}

int oct_to_dec(int n) {
    int res = 0;
    int index = 0;
    while (n != 0) {
        res += my_pow(8, index) * (n % 10);
        index++;
        n /= 10;
    }

    return res;
}

int my_pow(int base, int pow) {
    int res = 1;
    if (pow <= 0) {
        return 1;
    }
    while (pow > 0) {
        res *= base;
        pow--;
    }

    return res;
}
