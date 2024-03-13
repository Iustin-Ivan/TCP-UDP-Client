#ifndef _HELPERS_H
#define _HELPERS_H 1

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

typedef struct packet {
    char topic[50]={0};
	uint8_t type;
	char payload[1601]={0};
} packet;

#endif