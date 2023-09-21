CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

# List your source files here
SOURCES = fifo.c lru.c vms.c memsim.c

# Define the object file names based on source file names
OBJECTS = $(SOURCES:.c=.o)

# The name of the final executable
TARGET = memsim

# Default rule to build the target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to compile C source files into object files
%.o: %.c memsim.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove object files and the executable
clean:
	rm -f $(OBJECTS) $(TARGET)
