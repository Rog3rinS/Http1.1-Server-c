CC = gcc
CFLAGS = -Wall -g

SRCS = server.c http_handler.c
TARGET = server

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)
