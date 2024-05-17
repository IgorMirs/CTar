# Set the default compiler to gcc
CC = gcc

# Set the flags
CFLAGS = -Wall -Wextra -Werror -fsanitize=address -g3

# Files to run
SRC = main.c my_string.c my_tar_params.c my_tar_header.c
TARGET = my_tar
OBJ = main.o my_string.o my_tar_params.o my_tar_header.o

all : $(TARGET) clean

$(TARGET) : $(OBJ)
	gcc $(CFLAGS) -o $(TARGET) $(OBJ) 

$(OBJ) : $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)

clean:
	rm -f *.o

fclean: clean
	rm -f $(TARGET)

re: fclean all	