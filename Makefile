TARGET = shell
SRC_DIR = src
SRC = main.c parser.c helpers.c builtins.c executor.c string_helpers.c
CFLAGS = -Wall -Wextra -Werror
CC = gcc

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(addprefix $(SRC_DIR)/, $(SRC))
clean:
	rm -f $(TARGET)
fclean: clean
	rm -f $(TARGET)
re : fclean all
run : all
	./$(TARGET)