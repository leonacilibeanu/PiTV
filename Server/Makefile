CC = gcc
CFLAGS = -Wall

build: server

server: main.o sock_util.o
	$(CC) $(CFLAGS) $^ -o $@ -lrt

main.o: main.c

sock_util.o: sock_util.c

clean:
	rm *.o server
