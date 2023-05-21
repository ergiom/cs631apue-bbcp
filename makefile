CC = gcc
CFLAGS = -Wall -Werror -Wextra -ggdb

all: clean bbcp.c bbcp.h
	$(CC) $(CFLAGS) bbcp.c -o ./target/bbcp

clean:
	rm -f bbcp