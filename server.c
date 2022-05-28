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
	int sockfd = -1, connfd = -1, client_num = -1;
    struct sockaddr_in servaddr, clientaddr;
	socklen_t addrlen;
	int MAX = 1000;
	char buff[MAX];
	char* resp = "thanks for the message\n";
	int clients[1000];
	int MAX_NUM_CONNS = 100;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed.\n");
        exit(0);
    }
	
    memset(&servaddr, '\0', sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(5000);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("Bind failed...\n");
        exit(0);
    }
   
    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed.\n");
        exit(0);
    }
	
	while(1)
    {
        addrlen = sizeof(clientaddr);
        connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen);
		clients[client_num] = connfd;

        if (clients[client_num] < 0)
        {
            perror("accept() failed.\n");
			continue;
        }
     
        memset(buff, '\0', MAX);

		read(connfd, buff, sizeof(buff));

		// and send that buffer to client
		write(connfd, resp, strlen(resp));

		// After chatting close the socket
		close(sockfd);			
						
						
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