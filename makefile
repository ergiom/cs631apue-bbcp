CC = gcc
CFLAGS = -Wall -Werror -Wextra -ggdb
TARGET_DIR = ./target

all: clean bbcp.c bbcp.h
	$(CC) $(CFLAGS) bbcp.c -o $(TARGET_DIR)/bbcp

clean:
	rm -rf $(TARGET_DIR)/*
