#include <bits/stdc++.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <limits.h>
#include <math.h>
#include <vector>
#include <queue>
#include <map>
#include "helpers.h"

using namespace std;

typedef struct topicT{
	char name[51];
	bool sf;
	vector<packet> packets;
} topicT;

typedef struct Tclient {
	int descriptor;
	bool online;

    vector<topicT> messages;

} Tclient;

// fiecare socket are un string asociat
map<int, string> clients_ids;
// fiecare client are un Tclient asociat
map<string, Tclient> client_data;

int socketTCPserver;
int socketUDPserver;
int sockMax;
fd_set clients_descriptors;

void initialisations(char *portServer) {
	// initializare socketi
	socketTCPserver = socket(AF_INET, SOCK_STREAM, 6);
	socketUDPserver = socket(AF_INET, SOCK_DGRAM, 17);
	DIE(socketTCPserver < 0, "SOCKET SERVER FAILED");
	DIE(socketUDPserver < 0, "SOCKET SERVER FAILED");
    
	uint16_t port = atoi(portServer);
	DIE(port < 0, "Negative port / atoi error");

    // adrese IP
	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = 0;

    
	int NegleActivated = 1, res;
	// deactivate Negle, all my homies hate Negle
	res = setsockopt(socketTCPserver, 6, TCP_NODELAY, &NegleActivated, 4);
	DIE(res < 0, "Failed to deactivate Negle");
	int enable = 1;
	// asta e ca sa pot rula de mai multe ori serverul pe acelasi port
    if (setsockopt(socketTCPserver, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
	enable = 1;
	if (setsockopt(socketUDPserver, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

	res = bind(socketUDPserver, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (res < 0) {
       cout << "Bind failed for UDP port " << socketUDPserver << endl;
	}
	res = bind(socketTCPserver, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (res < 0) {
       cout << "Bind failed for TCP port << " << socketTCPserver << endl;
	}
	res = listen(socketTCPserver, INT32_MAX);
	if (res < 0) {
		cout << "Couldn't listen on TCP port " << socketTCPserver << endl;
	}
	sockMax = max(socketTCPserver, socketUDPserver);
}

void closeServer() {
	int res;
	// inchid toti socketii la final
	for (int i = 3; i <= sockMax; i++)
	{
		if (FD_ISSET(i, &clients_descriptors) && (i != socketTCPserver) && (i != socketUDPserver)) {
			res = close(i);
			if (res < 0) {
				cout << "Couldn't close socket " << i << endl;
			}
		}
	}
	res = close(socketTCPserver);
	DIE (res < 0, "Couldn't close socketTCPserver");
	res = close(socketUDPserver);
	DIE (res < 0, "Couldn't close socketUDPserver");
	exit(0);
}

int type0(char *payload, char sign, char* type_id) {
	// daca primesc mesaj de tip 1 de la UDP client bag formula
	int a;
    memcpy(&a, payload+1, sizeof(uint32_t));
        a = ntohl(a);
        if (sign) {
			a = -a;
		}
		return a;
}

double format_payload(uint8_t type, char* payload, char sign) {
	// Daca e de tip 1 sau 2, bag formula
	double signedint = 0;
	double shortreal = 0;
	double realnr = 0;
	uint32_t signedintint = 0;
	uint16_t shortrealreal = 0;
	uint32_t realnrnr = 0;
	switch (type)
	{
	case 1:
		memcpy(&shortrealreal, payload, sizeof(uint16_t));
		shortrealreal = ntohs((uint16_t)shortrealreal);
		shortreal = (double)shortrealreal / 100;
		return shortreal;

	case 2:
		memcpy(&realnrnr, payload+1, sizeof(uint32_t));
		realnrnr = ntohl((uint32_t) realnrnr);

		uint8_t power;
		memcpy(&power, payload+5, 1);
		realnr = (double)realnrnr / powf(10, power);
		if (sign) {
			realnr = -realnr;
		}
	    return realnr;
	
	default:
		return 0;
	}

}

void run() {

	fd_set aux;
    // initializare structura
	FD_ZERO(&clients_descriptors);
	FD_ZERO(&aux);

	FD_SET(socketTCPserver, &clients_descriptors);
	FD_SET(socketUDPserver, &clients_descriptors);
	FD_SET(0, &clients_descriptors);

	while(1) {
		aux = clients_descriptors;
		// de fiecare data cand primesc ceva, fac select cu aux ca sa nu modific
		// originalul

		int res = select(sockMax + 1, &aux, NULL, NULL, NULL);
		DIE(res < 0, "No message\n");
        for (int i = 0; i < sockMax + 1; i++) {
			// tot ce pot sa scriu la server e exit, altfel nu e valid
			if (FD_ISSET(i, &aux)) {
				char *buffer = (char*)calloc(1, sizeof(struct packet));
				DIE (buffer == NULL, "Failed to allocate memory");
				if (i == 0) {
					if (fgets(buffer, 1600, stdin) == NULL) {
					    cout << "Couldn't read from keyboard";
				    }
					if (strncmp(buffer, "exit", 4) == 0) {
					    closeServer();
					} else {
						cout<< "Invalid command\n";;
					}
				} else if (i == socketTCPserver) {
					// daca vreau un client tcp nou sa vina il accept
                    struct sockaddr_in addrclient;
					socklen_t len = sizeof (struct sockaddr_in);
                    int client_descriptor = accept(socketTCPserver, (struct sockaddr*)&addrclient, &len);
	                DIE(client_descriptor < 0, "New connection failed TCP\n");
                    // F Negle
		            int enable = 1;
		            int res = setsockopt(client_descriptor, SOL_TCP, TCP_NODELAY, (void *) &enable, sizeof(int));
		            DIE(res < 0, "Failed to deactivate Negle");
                    
					char buf[11];
					// id-ul clientului
					res = recv(client_descriptor, buf, 10, 0);
					DIE(res < 0, "Failed to read from socketTCP");
                    // verific daca e prima oara cand se conecteaza sau daca e deconectat si il pun online
					bool found = false, live = false;
					if (client_data[buf].online) {
						found = true;
						live = client_data[buf].online;
					} else if (client_data[buf].descriptor == 0) {
						found = true;
						live = false;
					}

					if (!found) {
						// daca e prima oara il bad in lista de client cu id-ul lui
						FD_SET(client_descriptor, &clients_descriptors);
				        sockMax = max(sockMax, client_descriptor);
					    clients_ids[client_descriptor] = buf;
						client_data[buf].descriptor = client_descriptor;
				        client_data[buf].online = true;
						cout << "New client " << buf << " connected from " << inet_ntoa(addrclient.sin_addr) 
						<<":"<< ntohs(addrclient.sin_port)<<"\n";
					} else if (found && !live) {
						// daca se reconecteaza verific si ce mesaje mai are intre timp cu sf on
						// si il adaug cu id-ul nou
						client_data[buf].descriptor = client_descriptor;
				        client_data[buf].online = true;
						FD_SET(client_descriptor, &clients_descriptors);
				        sockMax = max(sockMax, client_descriptor);
					    clients_ids[client_descriptor] = buf;
                        cout << "New client " << buf << " connected from " << inet_ntoa(addrclient.sin_addr) 
						<<":"<< ntohs(addrclient.sin_port)<<"\n";
						// daca are sf on atunci ii trimit mesajele pe care le avea
						for (int k = 0; k < client_data[buf].messages.size(); k++) {
							if (client_data[buf].messages[k].sf) {
						        vector<packet> vec = client_data[buf].messages[k].packets;
						        for (int j = 0; j < vec.size(); j++) {
							        res = send(client_data[buf].descriptor, (void *)&vec[j], sizeof(struct packet), 0);
							        DIE(res < 0, "Couldn't send message");
						        }
		
								client_data[buf].messages[k].packets.resize(0);
							}
						}
					} else {
						// daca e deja conectat il refuz
						close(client_descriptor);
						client_data[buf].online = false;
						cout << "Client "<< buf <<" already connected."<<"\n";
					}
				} else if (i == socketUDPserver) {
                    struct sockaddr_in addrclient;
					socklen_t len = sizeof (struct sockaddr_in);
                    int res = recvfrom(socketUDPserver, buffer, sizeof(struct packet), 0, (struct sockaddr *)&addrclient, &len);
					DIE(res < 0, "Failed to receive from UDP");

					struct packet *pack = (struct packet *) buffer;

		            // salvez port-ul si ip-ul clientului UDP care trimite mesaje
					uint16_t port = htons(addrclient.sin_port);
					char ip[16] = {0};
					char topic_name[51] = {0};
					memcpy(ip, inet_ntoa(addrclient.sin_addr), strlen(inet_ntoa(addrclient.sin_addr)));
					memcpy(topic_name, pack->topic, 50);
					char type_id[11] = {0};
					char fullmessage[1601] = {0};
					char payload_restrict[1501] = {0};
					memcpy(payload_restrict, pack->payload, 1500);
					int d = pack->type;
					// pentru fiecare tip am o chestie diferita de afisat
					// payload restrict e ca sa nu am warning la compilare
					if (d == 0) {
                        int nr = type0(pack->payload, pack->payload[0], type_id);
						sprintf(fullmessage, "%s:%hu - %s - %s - %d", ip, port, topic_name, "INT", nr);
					} else if (d == 1) {
						double nr = format_payload(pack->type, pack->payload, pack->payload[0]);
						sprintf(fullmessage, "%s:%hu - %s - %s - %.2f", ip, port, topic_name, "SHORT_REAL", nr);
					} else if (d == 2) {
						double nr = format_payload(pack->type, pack->payload, pack->payload[0]);
						sprintf(fullmessage, "%s:%hu - %s - %s - %lf", ip, port, topic_name, "FLOAT", nr);
					} else {
						sprintf(fullmessage, "%s:%hu - %s - %s - %s", ip, port, topic_name, "STRING", payload_restrict);
					}
					// vad ce clienti tcp sunt abonati la topicul respectiv si in functie daca
					// sunt on sau daca sunt off si au sf on trimit mesajul sau il salvez
					for (int j = 5; j < sockMax+1; j++) {
						string id = clients_ids[j];
						for (int k = 0; k < client_data[id].messages.size(); k++) {
							if (strncmp(client_data[id].messages[k].name, topic_name, strlen(topic_name)) == 0) {
                                if (client_data[id].online) {
									packet p;
									memcpy(p.payload, fullmessage, 1600);
								    int res = send(client_data[id].descriptor, &p, sizeof(struct packet), 0);
								    DIE (res < 0, "Couldn't send to the clients\n");
							    } else if (client_data[id].messages[k].sf) {
									packet packk;
									memcpy(packk.topic, topic_name, 50);
									packk.type = pack->type;
									memcpy(packk.payload, fullmessage, 1600);
									client_data[id].messages[k].packets.push_back(packk);
								}
						    }
						}
					}
				} else {
                    int res = recv(i, buffer, sizeof(struct packet), 0);
					DIE (res < 0, "Failed to receive from socketTCP");
					struct packet *package = (struct packet *) buffer;
					Tclient client;
                    client = client_data[clients_ids[i]];
					vector<topicT> topicsss = client.messages;
                    // daca un client vrea sa se deazboneze atunci creez un nou vector de topics
					// fara topicul respectiv si o atribui hashmap-ului global
					// altfel daca e subscribe il adaug la vectorul de topics
					bool found = false;
					if (strncmp(package->payload, "subscribe", 9)==0 || strncmp(package->payload, "unsubscribe", 11)==0) {
						for (int j = 0; j < topicsss.size(); j++) {
							if (strncmp(topicsss[j].name, package->topic, strlen(package->topic)) == 0) {
                               if (strncmp(package->payload, "unsubscribe", 11)==0) {
								   auto aux = topicsss[j];
								   vector<topicT> aux2;
								   for (int k = 0; k < topicsss.size(); k++) {
                                      if (strncmp(topicsss[k].name, aux.name, 50)) {
                                          aux2.push_back(topicsss[k]);
									  }
								   }
								   client_data[clients_ids[i]].messages = aux2;
								   found = true;
								   break;
							   }
							}
						}
						if (strncmp(package->payload, "subscribe", 9)==0 && !found) {
							topicT newTopic;
							memcpy(newTopic.name, package->topic, 50);
							newTopic.sf = package->type;
							topicsss.push_back(newTopic);
							client_data[clients_ids[i]].messages = topicsss;
					    }
						// daca vrea sa se deconecteze atunci il scot din hashmap si din fd_set
						// si inchid socketul si il setez offline cu descriptorul 0
					} else if (strncmp(package->payload, "exit", 4)==0) {
						FD_CLR(i, &clients_descriptors);
						client_data[clients_ids[i]].online = false;
						client_data[clients_ids[i]].descriptor = 0;
						cout << "Client " << clients_ids[i] << " disconnected.\n"; 
						close(i);
					}
 
			    }
		    }
	    }

    }
}

int main(int argc, char **argv)
{
	DIE(argc < 2, "Too few arguments");
	int res = setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	DIE(res != 0, "SETVBUF FAILED");

	initialisations(argv[1]);
    
	run();

	return 0;
}