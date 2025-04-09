# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -g -Wall -std=c99 -fsanitize=address -fsanitize=undefined

# Source files
SRCS = mysh.c

# Object files
OBJS = mysh.o

# Executable name
TARGET = mysh

# Default target to build all specified targets
all: 
	$(CC) $(CFLAGS) -o mysh $(SRCS)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)