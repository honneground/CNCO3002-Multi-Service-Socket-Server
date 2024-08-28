CC = gcc
CFLAGS = -Wall

COMMON_DEPS = toolKits.c

all: server client

server: server.c $(COMMON_DEPS)
	$(CC) $(CFLAGS) -o server server.c $(COMMON_DEPS)

client: client.c $(COMMON_DEPS)
	$(CC) $(CFLAGS) -o client client.c $(COMMON_DEPS)

clean:
	rm -f server client