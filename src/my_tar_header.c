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





void init_my_tar_header(my_tar_header* tar_head_ptr, my_tar_params* tar_param_ptr) {
    //no files to init
    if(tar_param_ptr->n_files == 0) {
        tar_head_ptr->isValidHeader = false;
        return;
    }
    struct stat* buf = (struct stat*) malloc(sizeof(struct stat));
    //invalid name of a file or directory
    if (lstat(tar_param_ptr->file_names[0], buf) != 0) {
        printf("my_tar: %s: Cannot stat: No such file or directory", tar_param_ptr->file_names[0]);
        tar_head_ptr->isValidHeader = false;
        return;
    }
    //valid header
    tar_head_ptr->isValidHeader = true;

    //set name
    set_name(tar_head_ptr, tar_param_ptr->file_names[0]);
    //set mode
    //% 512 (778 in octal) to consider only three last digits in mode
    init_str(tar_head_ptr, buf->st_mode % 01000, tar_head_ptr->mode, sizeof(tar_head_ptr->mode) / sizeof(char));  
    //set uid
    init_str(tar_head_ptr, buf->st_uid, tar_head_ptr->uid, sizeof(tar_head_ptr->uid)/ sizeof(char));
    //set gid
    init_str(tar_head_ptr, buf->st_gid, tar_head_ptr->gid, sizeof(tar_head_ptr->gid)/ sizeof(char));
    //set size
    my_puts("HERE1");
    init_str(tar_head_ptr, buf->st_size, tar_head_ptr->size, sizeof(tar_head_ptr->size)/ sizeof(char));
    my_puts("HERE2");
    //set mtime
    init_str(tar_head_ptr, buf->st_mtim.tv_sec, tar_head_ptr->mtime, sizeof(tar_head_ptr->mtime)/ sizeof(char));
    

    //set typeflag
    set_typeflag(tar_head_ptr, buf->st_mode); //TODO

    //set linkname
    set_linkname(tar_head_ptr);

    //set magic
    my_strcpy(tar_head_ptr->magic, "ustar");
    my_strcat(tar_head_ptr->total_header, tar_head_ptr->magic);

    //set version
    my_strcpy(tar_head_ptr->version, " ");
    my_strcat(tar_head_ptr->total_header, tar_head_ptr->version);


    //set uname
    set_uname(tar_head_ptr, buf->st_uid);
    
    //set gname
    set_gname(tar_head_ptr, buf->st_gid);
    
    //set checksum
    init_str(tar_head_ptr, count_chksum(tar_head_ptr), tar_head_ptr->chksum, sizeof(tar_head_ptr->chksum) / sizeof(char) -1 );
    char temp[] = " ";
    my_strcat(tar_head_ptr->chksum, temp);

    free(buf);     
}


void set_name(my_tar_header* tar_head_ptr, char* name) {
    my_strcpy(tar_head_ptr->name, name);
    //add to total
    my_strcpy(tar_head_ptr->total_header, name);
}

void set_typeflag(my_tar_header* tar_head_ptr, unsigned mode) {
    //regular file
    if (S_ISREG(mode)) {
        tar_head_ptr->typeflag = '0'; 
    }
    else if (S_ISLNK(mode)){
        tar_head_ptr->typeflag = '2';
    }

    //add to total
    char temp[2];
    temp[0] = tar_head_ptr->typeflag;
    temp[1] = '\0';
    my_strcat(tar_head_ptr->total_header, temp);
}

void set_linkname(my_tar_header* tar_head_ptr) {
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
    my_strcat(tar_head_ptr->total_header, tar_head_ptr->linkname);

    free(buf);
}

void set_uname(my_tar_header* tar_head_ptr, unsigned uid) {
    struct passwd *pwd;

    pwd = getpwuid(uid);
    if (pwd != NULL) {
        my_strcpy(tar_head_ptr->uname, pwd->pw_name);
    }
    else {
        tar_head_ptr->uname[0] = '\0';
    }
    
    //add to total
    my_strcat(tar_head_ptr->total_header, tar_head_ptr->uname);
}

void set_gname(my_tar_header* tar_head_ptr, unsigned gid) {
    struct group *grp;

    grp = getgrgid(gid);
    if (grp != NULL) {
        my_strcpy(tar_head_ptr->gname, grp->gr_name);
    }
    else {
        tar_head_ptr->gname[0] = '\0';
    }

    //add to total
    my_strcat(tar_head_ptr->total_header, tar_head_ptr->gname);
}

unsigned int count_chksum(my_tar_header* tar_head_ptr) {
    int sum = 0; 

    for(int i = 0; i < my_strlen(tar_head_ptr->total_header); i++) {
        sum += tar_head_ptr->total_header[i];
    }

    return sum;
}

void init_str(my_tar_header* tar_head_ptr, unsigned n, char* str, long unsigned size) {
    int i = 0;
    if (n == 0) { //the parameter is 0
        str[0] = '\0';
    }
    else {
        octal_to_str(n, str, &i);
    }
    add_leading_zeros(str, size);

    //add to total
    my_strcat(tar_head_ptr->total_header, str);
}

void print_tar_header(my_tar_header* tar_head_ptr) {
    if (!tar_head_ptr->isValidHeader) {
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
    printf("%s", tar_head_ptr->version);
    printf("%s", tar_head_ptr->uname);
    printf("%s", tar_head_ptr->gname);

}

void octal_to_str(unsigned n, char* str, int* i) {
    if (n == 0) {
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
    printf("my_strlen(str) %d\n", my_strlen(str));
    printf("n_zeros %d\n", n_zeros);
    if (n_zeros <= 0) {
        return;
    }
    my_puts("HERE4");
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


