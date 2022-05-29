#include<stdio.h>
#include<time.h>
#include<stdint.h>
#include<netdb.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<sys/select.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>

//smallest prime > 1024*512 (table size ~512MB)
#define NUM_ROWS 524309
#define KEY_SIZE 32
#define DATA_SIZE 1024
//since data is array of uint64_t (ie 8 bytes)
#define ELEMENTS_PER_ROW DATA_SIZE / 8

uint32_t hash_func(const uint8_t* key, int len, const uint8_t* iter);
void timing();
void serve();
void int_handler(int s);

//CACHE
// uint32_t size; // number of elements currently in cache
// uint32_t keys[NUM_ROWS];
// uint64_t data[NUM_ROWS * ELEMENTS_PER_ROW];
// uint16_t lens[NUM_ROWS]; // -1 if row is empty

int keep_running = 1;

int main(void){
	signal(SIGINT, int_handler);
	serve();
	return 0;
}

void int_handler(int s) {
    keep_running = 0;
	printf("\nShutting down server\n");
}

void serve(){
	//socket
	struct addrinfo hints, *res, *p;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	int option;
	char* port = "5000";
	int MAX_NUM_CONNS = 5;
	int server_fd;
	int clients[MAX_NUM_CONNS];
	int backlog = 1000; //max listen queue length
	fd_set readable;
    int client_num = 0;
	int BUFFER_SIZE = 1024;
	uint8_t buffer[BUFFER_SIZE];
	int message_size;
	char* resp = "X";
	int select_response;
	int current_client_fd;
	int max_fd;

    //get addrinfo for host
    memset(&hints, 0, sizeof(hints)); //initialize struct to 0
    hints.ai_family = AF_INET; //support iv4 ip addresses
    hints.ai_socktype = SOCK_STREAM; //TCP protocol
    hints.ai_flags = AI_PASSIVE; //indicates addr will be used in bind()

    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        printf("getaddrinfo() failed.\n");
        exit(1);
    }

    //bind socket to port
    for (p = res; p != NULL; p = p->ai_next)
    {
        option = 1;
        server_fd = socket(p->ai_family, p->ai_socktype, 0);
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        //socket creation failed
        if (server_fd == -1) 
        {
            continue;
        }

        //bind successful
        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == 0)
        {
            break;
        }
    }

    if (p == NULL)
    {
        printf("socket() or bind() failed.\n");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if (listen(server_fd, backlog) != 0)
    {
        printf("listen() failed.\n");
        exit(1);
    }
	
	FD_ZERO(&readable);
	FD_SET(server_fd, &readable);
	
	//necessary?
	int flag = fcntl(server_fd, F_GETFL, 0);
	flag |= O_NONBLOCK;
	fcntl(server_fd, F_SETFL, flag);
	
	for (int i = 0; i < MAX_NUM_CONNS; i++)
	{
		clients[i] = -1;
	}
    
    printf("Server started at http://127.0.0.1:%s\n", port);

    while(keep_running)
    {
		max_fd = server_fd + 1;
		
		for (int i = 0; i < MAX_NUM_CONNS; i++){
			if (clients[i] >= max_fd){
				max_fd = clients[i] + 1;
			}
		}
		
		//not really sure why I have to do this everytime
		FD_SET(server_fd, &readable);
		select_response = select(max_fd, &readable, NULL, NULL, NULL);
		
		if (select_response <= 0){
			printf("select() failed.\n");
			exit(1);
		}
		
		// accept new connection from server socket
		if (FD_ISSET(server_fd, &readable)) {
			//find empty spot for next client if possible
			client_num = 0;
			if (clients[client_num] != -1){
				for (int i = 0; i < MAX_NUM_CONNS; i++)
				{
					if (clients[i] == -1){
						client_num = i;
						break;
					}
				}
			}
			
			if (clients[client_num] == -1){
				memset(&clientaddr, 0, sizeof(clientaddr));
				addrlen = sizeof(clientaddr);
				clients[client_num] = accept(server_fd, (struct sockaddr *)&clientaddr, &addrlen);

				if (clients[client_num] < 0){
					printf("Accept() failed.\n");
					clients[client_num] = -1;
				}
				else {
					FD_SET(clients[client_num], &readable);
				}
			}
        }
		
		// read from clients
		for (int i = 0; i < MAX_NUM_CONNS; i++)
		{			
			if (clients[i] == -1){
				continue;
			}
			
			current_client_fd = clients[i];
			
			if (FD_ISSET(current_client_fd, &readable)){
				memset(buffer, '\0', BUFFER_SIZE);
				message_size = recv(current_client_fd, buffer, BUFFER_SIZE, 0);
				
				if (message_size <= 0){
					// disconnect client
					clients[i] = -1;
					close(current_client_fd);
					FD_CLR(current_client_fd, &readable);	
					continue;
				}	
				
				//parse message
				//reply resp = hashtable location
				int success = write(current_client_fd, resp, strlen(resp));
				if (success < 0){
					// disconnect client
					clients[i] = -1;
					close(current_client_fd);
					FD_CLR(current_client_fd, &readable);
				}
				else{
					printf("Responded with %s\n", resp);
				}
			} // FD_ISSET
		} //for i in (0, MAX_NUM_CONNS-1) 
    } //while(1)
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