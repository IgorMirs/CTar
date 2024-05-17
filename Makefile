#Compiler
CC = gcc-12

#The Target Binary Program
TARGET = my_zsh

#The Directories
SRC_DIR = ./src
BUILD_DIR = ./build

#Set the flags
CFLAGS = -Wall -Wextra -Werror 
#-fsanitize=address -g3

#Include dir
INC = -Iinc

#Search for .c files in the source directory
SRCS = $(shell find $(SRC_DIR) -type f -name "*.c")
#Replace the dir name and file extensions for object files
OBJS = $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRCS:.c=.o))

# Default make
all : $(TARGET)

#Link
$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

#Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(INC) -c $< -o $@

#Clean only Objects
clean:
	rm -f $(BUILD_DIR)/*.o

fclean: clean
	rm -f $(TARGET)

re: fclean all	
