# Compiler
CC = gcc

# Compiler flags
CFLAGS = -g -Wall -std=c99 -fsanitize=address -fsanitize=undefined

# Source and header files
SRCS = mysh.c
OBJS = mysh.o token.o
HEADERS = token.h commands.h

# Executable name
TARGET = mysh

# Default build
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile each .c file into a .o file
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(TARGET)
