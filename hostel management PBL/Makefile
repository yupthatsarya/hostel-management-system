# Compiler settings - standard ANSI C compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c89 -pedantic -O2

# Project name
TARGET = hostel_system

# Source files
SRCS = main.c utils.c auth.c student.c allotment.c room.c reports.c audit.c server.c
OBJS = $(SRCS:.c=.o)

# Cross-platform support for cleaning build artifacts and linking sockets
ifeq ($(OS),Windows_NT)
    RM = del /F /Q
    TARGET_BIN = $(TARGET).exe
    CLEAN_OBJS = $(subst /,\\,$(OBJS))
    LIBS = -lws2_32
else
    RM = rm -f
    TARGET_BIN = $(TARGET)
    CLEAN_OBJS = $(OBJS)
    LIBS =
endif

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET_BIN) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-$(RM) $(CLEAN_OBJS) $(TARGET_BIN)
else
	-$(RM) $(OBJS) $(TARGET_BIN)
endif

.PHONY: all clean
