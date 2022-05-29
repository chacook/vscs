#include<stdio.h>
#include<time.h>
#include<stdint.h>

#include <netdb.h>
//#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

uint32_t hash_func(const uint8_t* key, int len, const uint8_t* iter);
void timing();
void serve();

//smallest prime > 1024*512 (table size ~512MB)
#define NUM_ROWS 524309
#define KEY_SIZE 32
#define DATA_SIZE 1024
//since data is array of uint64_t (ie 8 bytes)
#define ELEMENTS_PER_ROW DATA_SIZE / 8

//CACHE
// uint32_t size; // number of elements currently in cache
// uint32_t keys[NUM_ROWS];
// uint64_t data[NUM_ROWS * ELEMENTS_PER_ROW];
// uint16_t lens[NUM_ROWS]; // -1 if row is empty

int main(void){
	serve();
	return 0;
}

void serve(){
	char* port = "5000";
	int MAX_NUM_CONNS = 100;
	int listen_fd, clients[MAX_NUM_CONNS];
	struct addrinfo hints, *res, *p;
    int option;
	struct sockaddr_in clientaddr;
    socklen_t addrlen;   
    int client_num = 0;
	int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];
	int message_size;
	int backlog = 1000; //max listen queue length
	int client_fd;
	char* resp = "X";

    //get addrinfo for host
    memset(&hints, 0, sizeof(hints)); //initialize struct to 0
    hints.ai_family = AF_INET; //support iv4 ip addresses
    hints.ai_socktype = SOCK_STREAM; //TCP protocol
    hints.ai_flags = AI_PASSIVE; //indicates addr will be used in bind()

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() failed.\n");
        exit(1);
    }

    //bind socket to port
    for (p = res; p != NULL; p = p->ai_next)
    {
        option = 1;
        listen_fd = socket(p->ai_family, p->ai_socktype, 0);
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        //socket creation failed
        if (listen_fd == -1) 
        {
            continue;
        }

        //bind successful
        if (bind(listen_fd, p->ai_addr, p->ai_addrlen) == 0)
        {
            break;
        }
    }

    if (p == NULL)
    {
        perror("socket() or bind() failed.\n");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if (listen(listen_fd, backlog) != 0)
    {
        perror("listen() failed.\n");
        exit(1);
    }
    
    printf("Server started at http://127.0.0.1:%s\n", port);

    //initialize clients array (-1 = no client)
    for (int i = 0; i < MAX_NUM_CONNS; i++)
    {
        clients[i] = -1;
    }


    //accept connections
    while(1)
    {
        addrlen = sizeof(clientaddr);
        clients[client_num] = accept(listen_fd, (struct sockaddr *)&clientaddr, &addrlen);

        if (clients[client_num] < 0)
        {
            perror("accept() failed.\n");
			continue;
        }

		message_size = recv(clients[client_num], buffer, BUFFER_SIZE, 0);
		if (message_size < 0)
		{
			perror("recv() failed.\n");
			exit(1);
		}
		else if (message_size == 0)
		{
			perror("Client disconnected unexpectedly.\n");
			exit(1);
		}
		
		client_fd = clients[client_num];
		write(client_fd, resp, strlen(resp));

		//find empty spot for next client
		while (clients[client_num] != -1)
		{
			client_num = (client_num + 1) % MAX_NUM_CONNS;
		}
              
    }
   
}

//credit: http://www.azillionmonkeys.com/qed/hash.html
#ifndef get16bits
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

uint32_t hash_func(const uint8_t* key, int len, const uint8_t* iter){
	uint32_t hash = len, tmp;
	int rem;

    if (len <= 0 || key == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    // Main loop
    for (;len > 0; len--) {
        hash += get16bits (key);
        tmp = (get16bits (key+2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        key += 2*sizeof (uint16_t);
        hash += hash >> 11;
    }
	
	// me: Hopefully this is good enough?
	if (iter != NULL){
		hash += get16bits (iter);
		tmp = (get16bits (iter) << 11) ^ hash;
		hash = (hash << 16) ^ tmp;
		hash += hash >> 11;
	}

    // Handle end cases
    switch (rem) {
        case 3: hash += get16bits (key);
                hash ^= hash << 16;
                hash ^= ((signed char)key[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (key);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*key;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    // Force "avalanching" of final 127 bits
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

void timing(){
	clock_t start, end;
    double cpu_time_used;
     
    start = clock();

    uint8_t aaa = 0xde;
    printf("%u\n", hash_func(&aaa, 1, NULL) % 1667001);
     
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("%f\n", cpu_time_used);
}