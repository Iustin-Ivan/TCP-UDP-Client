#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "helpers.h"

using namespace std;

fd_set set_fd_uri;
int socketTCPserver;
struct sockaddr_in server_data;
int port;

// in principiu aceeasi chestie ca in server.cpp am explicat deja

void initialisations(char *portServer, char *ipServer, char *id) {
	port = atoi(portServer);

	socketTCPserver = socket(AF_INET, SOCK_STREAM, 0);
	DIE(socketTCPserver < 0, "socket er");

	int enable = 1;
	setsockopt(socketTCPserver, IPPROTO_TCP, TCP_NODELAY, (char *)&enable, sizeof(int));

	server_data.sin_port = htons(port);
	server_data.sin_family = AF_INET;
	
	inet_aton(ipServer, &server_data.sin_addr);

	FD_ZERO(&set_fd_uri);
	FD_SET(socketTCPserver, &set_fd_uri);
	FD_SET(STDIN_FILENO, &set_fd_uri);

	int ret = connect(socketTCPserver, (struct sockaddr *)&server_data, sizeof(server_data));
	DIE(ret < 0, "connect server");
	// trimit id-ul pentru a fi inregistrat in server
	ret = send(socketTCPserver, id, 10, 0);
	DIE(ret < 0, "send1");

}

int main(int argc, char** argv) {

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	DIE(argc < 4, "< arguments");
	int enable = 1;
	initialisations(argv[3], argv[2], argv[1]);
	struct packet pack;
	while(1) {
		fd_set fd_aux = set_fd_uri;

		int res = select(socketTCPserver + 1, &fd_aux, NULL, NULL, NULL);
        DIE (res < 0, "select");

		if (FD_ISSET(STDIN_FILENO, &fd_aux)) {
			char buffer[100];
			fgets(buffer, 100, stdin);

			buffer[strlen(buffer) - 1] = 0;
			char *command = strtok(buffer, " ");
			// tipurile de comenzi pe care le poate da subscriberul
			char *sf;
			switch (strlen(command)) {
				case 4:
					memcpy(pack.payload, command, 4);
                    res = send(socketTCPserver, &pack, sizeof(struct packet), 0);
					DIE(res < 0, "send failed");
					exit(0);
				case 9:
				    memcpy(pack.payload, command, 9);
					memcpy(pack.topic, strtok(NULL, " "), 50);
					sf = strtok(NULL, " ");
					pack.type = sf[0] - '0';
					res = send(socketTCPserver, &pack, sizeof(struct packet), 0);
					DIE(res < 0, "send failed");
					cout << "Subscribed to topic.\n";
					break;
				case 11:
					memcpy(pack.payload, command, 11);
					memcpy(pack.topic, strtok(NULL, " "), 50);
					res = send(socketTCPserver, &pack, sizeof(struct packet), 0);
					DIE(res < 0, "send failed");
					cout << "Unsubscribed from topic.\n";
					break;
				default:
				    cout << "Invalid command\n";
                    break;   
			}
		}
		// daca primeste ceva de la server ia mesajul din payload si il afiseaza
		if(FD_ISSET(socketTCPserver, &fd_aux)) { 

          
			char buffer[sizeof(struct packet)];

			int res = recv(socketTCPserver, buffer, sizeof(struct packet), 0);

			if(res <= 0) {
				exit(0);
			}
            
			struct packet *pack_send = (struct packet *)buffer;
			cout << pack_send->payload << "\n";
		}
	    }

    }