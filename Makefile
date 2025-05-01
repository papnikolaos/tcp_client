# ---- Makefile for tcp-echo-client ---------------------------------

# Compiler and common flags
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic -g          # add or remove flags as you prefer

# Output binary name
TARGET  := tcp-echo-client

# Source files (adjust if you add more .c files later)
SRC     := tcp-echo-client.c

# -------------------------------------------------------------------
# Default build target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Remove build artifacts
.PHONY: clean
clean:
	$(RM) $(TARGET)

# Re-build everything from scratch
.PHONY: rebuild
rebuild: clean $(TARGET)
