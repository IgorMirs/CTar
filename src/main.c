#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#include "my_tar_params.h"
#include "my_tar_header.h"
#include "my_string.h"

#ifndef TAR_HEAD_LIST_NODE
#define TAR_HEAD_LIST_NODE
struct tar_head_list_node{
    my_tar_header val;
    struct tar_head_list_node* next;
};
typedef struct tar_head_list_node tar_head_list_node;
#endif


/*********PROTOTYPES**********/

bool is_valid_input(my_tar_params* tar_p_ptr);
int add_end_of_archieve();
bool is_end_of_archieve(my_tar_header* tar_h_ptr);
void list_my_tar(char* archive_name);

bool is_t_flag(my_tar_params* tar_p_ptr);
bool is_c_flag(my_tar_params* tar_p_ptr);
bool is_x_flag(my_tar_params* tar_p_ptr);
bool is_r_flag(my_tar_params* tar_p_ptr);
bool is_u_flag(my_tar_params* tar_p_ptr);
int exec_t_flag(my_tar_params* tar_p_ptr);
int exec_c_flag(my_tar_params* tar_p_ptr);
int exec_x_flag(my_tar_params* tar_p_ptr);
int exec_r_flag(my_tar_params* tar_p_ptr);

int my_tar(my_tar_params* tar_p_ptr); 

int create_my_tar_file(my_tar_header* tar_h_ptr, int fd); 

char* get_file_name(char* file_path);

/*********IMPLEMENTATIONS**********/

tar_head_list_node* create_list_node(my_tar_header val) {
    tar_head_list_node* new_node = (tar_head_list_node*) malloc(sizeof(tar_head_list_node));
    new_node->val = val;
    new_node->next = NULL;

    return new_node;
}

void print_tar_list(tar_head_list_node* head) {
    tar_head_list_node* cur = head;
    while (cur) {
        print_tar_header(&cur->val);
        cur = cur->next;
    }
}

bool is_mod_time_newer(my_tar_header* h1, my_tar_header* h2) {
    return ((my_strcmp(h1->mtime, h2->mtime) < 0));
}

bool are_equal_files(my_tar_header* h1, my_tar_header* h2) {
    return ((my_strcmp(h1->name, h2->name) == 0) && 
            (my_strcmp(h1->mode, h2->mode) == 0) && 
            (h1->typeflag == h2->typeflag));
}

//1 - to add, 0 - not add
bool check_file_version(tar_head_list_node* head, my_tar_header* header) {
    tar_head_list_node* cur = head;
    while (cur) {
        if(are_equal_files(&cur->val, header) &&
            !is_mod_time_newer(&cur->val, header)) {
            return false;
        }
        cur = cur->next;
    }

    return true;
}



int create_my_tar_dir(my_tar_header* tar_h_ptr, int fd) { //TODO (think about recursive solution)
    //write dir info into the archive
    int wr_size = write(fd, tar_h_ptr, sizeof(my_tar_header));
    if (wr_size < 0) {
        char err_msg[] = "[ERROR] : write create_my_tar_dir\n";
        write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
        return 1; //error
    } 

    int exit_val = 0;
    int dir_name_length = my_strlen(tar_h_ptr->name);
    char* dir_name = (char*) malloc((dir_name_length + 1) * sizeof(char));
    my_strcpy(dir_name, tar_h_ptr->name);
    DIR* dr = opendir(tar_h_ptr->name);
    struct dirent *de;


    //iterate through the directory and add files to the archive
    while((de = readdir(dr)) != NULL) {
        if (de->d_name[0] != '.') {
            //create the file path
            int file_name_length = my_strlen(de->d_name);
            char* path = (char*) malloc((dir_name_length + file_name_length + 1) * sizeof(char));
            my_strcpy(path, dir_name);
            my_strcat(path, de->d_name);

            //create the header for a file in the directory
            int in_tar_head = init_my_tar_header(tar_h_ptr, path); 
            int cr_tar_file = create_my_tar_file(tar_h_ptr, fd);
            if (in_tar_head == 1 || cr_tar_file == 1) {
                exit_val = 1; //error
            }

            free(path);
        }
    }

    free(dir_name);

    closedir(dr);

    return exit_val;
}


int create_my_tar_file(my_tar_header* tar_h_ptr, int fd) {
    int fd_old = open(tar_h_ptr->name, O_RDONLY);
    int exit_val = 0;
    if (fd_old < 0) {
        char err_msg[] = "[ERROR] : open create_my_tar_file ";
        write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
        return 1; //error
    }
    
    //get the size of file
    unsigned size;
    struct stat* buf = (struct stat*) malloc(sizeof(struct stat));
    if (lstat(tar_h_ptr->name, buf) == 0) {
        size = buf->st_size;
    }
    else {
        size = 0;
    }
    free(buf);

    //read the file
    char *str = (char*) malloc(sizeof(char) * size + 1);
    int sz = read(fd_old, str, size); //returns the number of read bytes
    if (sz < 0) {
        char err_msg[] = "[ERROR] : read create_my_tar_file ";
        write(2, err_msg, sizeof(err_msg) / sizeof(char));
        exit_val = 1; //error 
    }
    else {
        str[sz] = '\0';
    }

    //write the header
    int wr_size = 0, wr_size2 = 0;
    if (exit_val == 0) {
        wr_size = write(fd, tar_h_ptr, sizeof(my_tar_header)); 
        //write the file
        wr_size2 = write(fd, str, size); 
        //add empty end of file
        unsigned end_size = (512 - (size % 512));
        if (end_size > 0) {
            char* end_of_file = (char*) malloc (end_size * sizeof(char));
            init_str_wzeros(end_of_file, end_size);
            wr_size2 = write(fd, end_of_file, end_size); 
            free(end_of_file);
        }
    }
    if (wr_size < 0 || wr_size2 < 0) {
        char err_msg[] = "[ERROR] : write create_my_tar_file ";
        write(2, err_msg, sizeof(err_msg) / sizeof(char));
        exit_val = 1; //error
    }
    
    free(str);
    close(fd_old);

    return exit_val;
}

int add_end_of_archieve(int fd) {
    //add the end of archive
    char end_of_archive[7680];
    init_str_wzeros(end_of_archive, sizeof(end_of_archive) / sizeof(char));
    int wr_size = write(fd, end_of_archive, sizeof(end_of_archive) / sizeof(char));
    if(wr_size < 0) {
        char err_msg[] = "[ERROR] : write add_end_of_archieve ";
        write(2, err_msg, sizeof(err_msg) / sizeof(char));
        return 1;
    }
    return 0;
}

bool is_end_of_archieve(my_tar_header* tar_h_ptr) {
    bool is_end = isZeroString(tar_h_ptr->name, sizeof(tar_h_ptr->name) / sizeof(char));
    if (is_end) {
        return true;
    }
    else {
        return false;
    }
}



int exec_t_flag(my_tar_params* tar_p_ptr) {
    int fd = open(tar_p_ptr->archive_name, O_RDONLY);
    if (fd < 0) {
        char err_msg[] = "[ERROR] : open read t_flag\n";
        write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
        return 1; //error
    }

    //read the file
    my_tar_header* tar_h_ptr = (my_tar_header*) malloc(sizeof(my_tar_header));
    
    int exit = 0;
    int exit_val = 0;
    while(exit != 2 && exit_val == 0) { //need to find to consecuite 512 zero bytes
        int sz = read(fd, tar_h_ptr, sizeof(my_tar_header));
        if (sz > 0 && is_end_of_archieve(tar_h_ptr)) {
            exit++;
        }
        else if(sz > 0) {
            //print names
            my_puts(tar_h_ptr->name);
            
            //skip the content of a file
            int size = oct_to_dec(strtoi(tar_h_ptr->size));
            size += 512 - (size % 512);
            char* temp = (char*) malloc((size + 1) * sizeof(char));

            sz = read(fd, temp, size);
            if (sz < 0) {
                exit_val = 1;
                char err_msg[] = "[ERROR] : read exec_t_flag ";
                write(2, err_msg, sizeof(err_msg) / sizeof(char));
            }

            free(temp);
        }
        else { //sz < 0
            char err_msg[] = "[ERROR] : in read t_flag\n";
            write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
            return 1; //error
        }
    } //end while

    free(tar_h_ptr);
    close(fd);

    return 0; //success
}

int exec_x_flag(my_tar_params* tar_p_ptr) {
    int exit_val = 0;
    my_tar_header* tar_h_ptr = (my_tar_header*) malloc(sizeof(my_tar_header));
    int fd_temp = open(tar_p_ptr->archive_name, O_RDONLY);
    if (fd_temp < 0) {
        exit_val = 1; //error
    } 
    
    int exit = 0;
    while(exit != 2 && exit_val == 0) { //need to find to consecuite 512 zero bytes
        int sz = read(fd_temp, tar_h_ptr, sizeof(my_tar_header));
        if (sz < 0) { //unsuccessfull read
            exit_val = 1;
            char err_msg[] = "[ERROR] : read exec_x_flag ";
            write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
            break;
        }
        else if (is_end_of_archieve(tar_h_ptr)) {
            exit++;
        }
        else {
            //get mode of the file header
            unsigned mode = oct_to_dec(strtoi(tar_h_ptr->mode));
            
            if(tar_h_ptr->typeflag == '5') { //if dir
                int status_mkdir = mkdir(tar_h_ptr->name, mode);
                if (status_mkdir <= 0 && exit_val == 0) {
                    exit_val = 1;
                    char err_msg[] = "[ERROR] : mkdir exec_x_flag\n";
                    write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
                }
            }
            else { //if file
                int fd = open(tar_h_ptr->name, O_CREAT | O_RDWR | O_TRUNC, mode); 
                if (fd < 0 && exit_val == 0) {
                    exit_val = 1;
                    char err_msg[] = "[ERROR] : open exec_x_flag\n";
                    write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
                    write(2, tar_h_ptr->name, my_strlen(tar_h_ptr->name));
                }
                //read the content of a file
                int size = oct_to_dec(strtoi(tar_h_ptr->size));
                size += 512 - (size % 512);
                char* file_content = (char*) malloc((size + 1) * sizeof(char));
                int r_size = read(fd_temp, file_content, size);
                if (r_size < 0 && exit_val == 0) {
                    exit_val = 1;
                    char err_msg[] = "[ERROR] : read exec_x_flag ";
                    write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
                }
                //write the file content                
                int w_size = write(fd, file_content, my_strlen(file_content)); 
                if (w_size < 0 && exit_val == 0) {
                    exit_val = 1;
                    char err_msg[] = "[ERROR] : write exec_x_flag ";
                    write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
                }
                
                //free memory
                close(fd);
                free(file_content);
            }
        }
    }

    free(tar_h_ptr);
    close(fd_temp);

    return exit_val;
}

int exec_r_flag(my_tar_params* tar_p_ptr) {
    int fd = open(tar_p_ptr->archive_name, O_RDONLY);
    int fd2 = open(tar_p_ptr->archive_name, O_RDWR);
    int exit_val = 0;

    if (fd < 0 || fd2 < 0) {
        exit_val = 1;
        char err_msg[] = "[ERROR] : open exec_r_flag ";
        write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
    }

    //read the file
    my_tar_header* tar_h_ptr = (my_tar_header*) malloc(sizeof(my_tar_header));
    my_tar_header* tar_h_ptr2 = (my_tar_header*) malloc(sizeof(my_tar_header));
    
    while(exit_val == 0) {
        int sz = read(fd, tar_h_ptr, sizeof(my_tar_header));
        if (sz < 0) {
            exit_val = 1;
            char err_msg[] = "[ERROR] : read exec_r_flag ";
            write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
            break;
        }
        else if (is_end_of_archieve(tar_h_ptr)) {
            //start writing into an archive
            for (int i = 0; i < tar_p_ptr->n_files; i++) {
                //init a header for a current file    
                if (init_my_tar_header(tar_h_ptr2, tar_p_ptr->file_names[i]) == 1) {
                    exit_val = 1;
                    break;
                } 
                //if the name is a directory
                int last_index = my_strlen(tar_h_ptr2->name) - 1;
                if (last_index > 0 && tar_h_ptr2->name[last_index] == '/') {
                    exit_val = create_my_tar_dir(tar_h_ptr2, fd2);
                }
                else {
                    //create the file
                    exit_val = create_my_tar_file(tar_h_ptr2, fd2);
                }
            }
            if (exit_val == 0) {
                exit_val = add_end_of_archieve(fd2);
            }

            break;
        }
        else { //not the end of archieve
            int r_size = read(fd2, tar_h_ptr2, sizeof(my_tar_header));
            if (r_size < 0) {
                exit_val = 1;
                char err_msg[] = "[ERROR] : read exec_r_flag ";
                write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
                break;
            }
            
            //skip the content of a file
            int size = oct_to_dec(strtoi(tar_h_ptr->size));
            //+ the end of file
            size += 512 - (size % 512); 
            char* temp = (char*) malloc((size + 1) * sizeof(char));
            
            //read into first file descriptor
            if ((r_size = read(fd, temp, size)) < 0) {
                exit_val = 1;
                char err_msg[] = "[ERROR] : read exec_r_flag ";
                write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
            }
            //read into second file descriptor
            else if ((r_size = read(fd2, temp, size)) < 0) {
                exit_val = 1;
                char err_msg[] = "[ERROR] : read exec_r_flag ";
                write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
            }

            free(temp);
        }
    } //end while

    free(tar_h_ptr);
    free(tar_h_ptr2);
    close(fd);
    close(fd2);

    return exit_val;
}

int exec_u_flag(my_tar_params* tar_p_ptr) {
    int fd_r = open(tar_p_ptr->archive_name, O_RDONLY);
    int fd_w = open(tar_p_ptr->archive_name, O_RDWR);
    int exit_val = 0;

    if (fd_r < 0 || fd_w < 0) {
        exit_val = 1;
        char err_msg[] = "[ERROR] : open exec_u_flag ";
        write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
    }

    //read the file
    my_tar_header* tar_h_ptr_r = (my_tar_header*) malloc(sizeof(my_tar_header));
    my_tar_header* tar_h_ptr_w = (my_tar_header*) malloc(sizeof(my_tar_header));
    
    tar_head_list_node* head = NULL;

    while(true && exit_val == 0) {
        int sz = read(fd_r, tar_h_ptr_r, sizeof(my_tar_header));
        if (sz < 0) {
            exit_val = 1;
            char err_msg[] = "[ERROR] : read exec_u_flag ";
            write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
            break;
        }
        else if (is_end_of_archieve(tar_h_ptr_r)) {
            //start writing into an archive
            for (int i = 0; i < tar_p_ptr->n_files; i++) {
                //init a header for a current file    
                if (init_my_tar_header(tar_h_ptr_w, tar_p_ptr->file_names[i]) == 1) {
                    exit_val = 1;
                    break;
                } 
                //check do we want to update the file
                if (check_file_version(head, tar_h_ptr_w) == true) {
                    //if the name is a directory
                    int last_index = my_strlen(tar_h_ptr_w->name) - 1;
                    if (last_index > 0 && tar_h_ptr_w->name[last_index] == '/') {
                        exit_val = create_my_tar_dir(tar_h_ptr_w, fd_w);
                    }
                    else {
                        //create the file
                        exit_val = create_my_tar_file(tar_h_ptr_w, fd_w);
                    }
                }            

            }
            if (exit_val == 0) {
                exit_val = add_end_of_archieve(fd_w);
            }

            break;
        }
        else { //not the end of archieve
            int r_size = read(fd_w, tar_h_ptr_w, sizeof(my_tar_header));
            if (r_size < 0) {
                exit_val = 1;
                char err_msg[] = "[ERROR] : read exec_u_flag ";
                write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
                break;
            }

            //create list of tar headers
            if (!head) {
                head = create_list_node(*tar_h_ptr_w);
            }
            else {
                tar_head_list_node* cur = head;
                while (cur->next != NULL) {
                    cur = cur->next;
                }
                cur->next = create_list_node(*tar_h_ptr_w);
            }
            
            //skip the content of a file
            int size = oct_to_dec(strtoi(tar_h_ptr_w->size));
            size += 512 - (size % 512);
            char* temp = (char*) malloc((size + 1) * sizeof(char));
            
            //read into first file descriptor
            if ((r_size = read(fd_r, temp, size)) < 0) {
                exit_val = 1;
                char err_msg[] = "[ERROR] : read exec_u_flag ";
                write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
            }
            //read into second file descriptor
            else if ((r_size = read(fd_w, temp, size)) < 0) {
                exit_val = 1;
                char err_msg[] = "[ERROR] : read exec_u_flag ";
                write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
            }

            free(temp);
        }
    } //end while
    //delete list of headers
    if (head) {
        tar_head_list_node* cur = head;
        while (cur) {
            tar_head_list_node* temp = cur;
            cur = cur->next;
            free(temp);
        }
    }


    free(tar_h_ptr_r);
    free(tar_h_ptr_w);
    close(fd_r);
    close(fd_w);

    return exit_val;
}


int exec_c_flag(my_tar_params* tar_p_ptr) {
    //create an archieve
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int fd = open(tar_p_ptr->archive_name, O_CREAT | O_RDWR | O_TRUNC, mode); 
    if (fd < 0) {
        char err_msg[] = "[ERROR] : open exec_c_flag\n";
        write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
        return 1; //error
    }
    int exit_value = 0;
    int last_index = 0;
    //create the header struct
    my_tar_header* tar_h_ptr = (my_tar_header*) malloc(sizeof(my_tar_header));
    //run the loop to go through all the files
    for (int i = 0; i < tar_p_ptr->n_files; i++) {
        //init a header for a current file
        if (init_my_tar_header(tar_h_ptr, tar_p_ptr->file_names[i]) == 1 ||
            isZeroString(tar_h_ptr->size, sizeof(tar_h_ptr->size) / sizeof(char))) {
            exit_value = 1; //error
            break;
        } 
        //if the name is a directory
        else if ((last_index = my_strlen(tar_h_ptr->name) - 1) > 0 && tar_h_ptr->name[last_index] == '/') {
            exit_value = create_my_tar_dir(tar_h_ptr, fd);
        }
        //if the name is a file
        else {
            exit_value = create_my_tar_file(tar_h_ptr, fd);
        }
    }
    if (exit_value == 0) {
        exit_value = add_end_of_archieve(fd);
    }

    free(tar_h_ptr);
    close(fd);

    return exit_value;
}

bool is_c_flag(my_tar_params* tar_p_ptr) { 
    //true if only f and c flags are specified
    return (tar_p_ptr->option_c && !tar_p_ptr->option_x 
            && !tar_p_ptr->option_t && !tar_p_ptr->option_r && !tar_p_ptr->option_u
            && tar_p_ptr->valid_flag);
}

bool is_t_flag(my_tar_params* tar_p_ptr) { 
    //true if only f and t flags are specified
    return (tar_p_ptr->option_t && !tar_p_ptr->option_c 
            && !tar_p_ptr->option_x && !tar_p_ptr->option_r && !tar_p_ptr->option_u
            && tar_p_ptr->valid_flag);
}

bool is_x_flag(my_tar_params* tar_p_ptr) {
    return (tar_p_ptr->option_x && !tar_p_ptr->option_c 
            && !tar_p_ptr->option_t && !tar_p_ptr->option_r && !tar_p_ptr->option_u
            && tar_p_ptr->valid_flag);
}

bool is_r_flag(my_tar_params* tar_p_ptr) {
    return (tar_p_ptr->option_r && !tar_p_ptr->option_c 
            && !tar_p_ptr->option_t && !tar_p_ptr->option_x && !tar_p_ptr->option_u 
            && tar_p_ptr->valid_flag);
}

bool is_u_flag(my_tar_params* tar_p_ptr) {
    return (tar_p_ptr->option_u && !tar_p_ptr->option_r && !tar_p_ptr->option_c 
            && !tar_p_ptr->option_t && !tar_p_ptr->option_x 
            && tar_p_ptr->valid_flag);
}

int my_tar(my_tar_params* tar_p_ptr) {
    if(is_t_flag(tar_p_ptr)) {
        return exec_t_flag(tar_p_ptr);
    }
    else if (is_c_flag(tar_p_ptr)) { 
        return exec_c_flag(tar_p_ptr);
    }
    else if (is_x_flag(tar_p_ptr)) {
        return exec_x_flag(tar_p_ptr);
    }
    else if (is_r_flag(tar_p_ptr)) {
        return exec_r_flag(tar_p_ptr);
    }
    else if (is_u_flag(tar_p_ptr)) {
        return exec_u_flag(tar_p_ptr);
    }
    else {
        char err_msg[] = "[ERROR] : INVALID FLAGS' COMBINATION!\n";
        write(2, err_msg, sizeof(err_msg) / sizeof(char)); 
        return 1; //error
    }

    return 0; //success
}

bool is_valid_input(my_tar_params* tar_p_ptr) {
    //no files to init
    
    if((tar_p_ptr->n_files == 0 && tar_p_ptr->archive_name == NULL) || tar_p_ptr->valid_flag == false) {
        return false;
    }
    else {
        return true;
    }
}


/*******************MAIN*******************/
int main(int ac, char** av) {
    my_tar_params* tar_p_ptr = (my_tar_params*) malloc(sizeof(my_tar_params));
    
    int exit_val = 1; //error by default

    //init parameters (flags, files)
    if (init_my_tar_params(tar_p_ptr, ac, av) == 0) {
        //run the main part if input parameters are valid
        exit_val = my_tar(tar_p_ptr); //exit_val could be 0 if there are no errors in my_tar
    }

    //free the memory   
    clean_my_tar_params(tar_p_ptr);
    free(tar_p_ptr);

    return exit_val;
}

