# Protocoale de comunicatii
# Laborator 7 - TCP
# Echo Server
# Makefile

CFLAGS = -Wall -std=c++17 -fsanitize=address -g -Werror -Wno-error=unused-variable

# Portul pe care asculta serverul
PORT = 12345

# Adresa IP a serverului
IP_SERVER = 127.0.0.1

ID = C1

all: server subscriber

server: server.cpp

subscriber: subscriber.cpp

.PHONY: clean run_server run_subscriber

run_server:
	./server ${PORT}

run_subscriber:
	./subscriber ${ID} ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
