#ifndef NETWORK_H
#define NETWORK_H
#define _XOPEN_SOURCE 600
#include<stdbool.h>
#include<stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct _net_ans{
    char* msg;
    int len;
}net_ans;

int s; //a network socket

struct addrinfo hints;
struct addrinfo *servinfo;  // will point to the results
struct timeval to;
struct addrinfo *mixer_IP;
struct sockaddr_storage their_addr;



void *get_addr(struct sockaddr *address);

/**
 * initializes the network in broadcast mode, that is needed to find the mixer
 */
void ini_bc_net();

/**
 * sends a broadcast message and listens for the mixer's answers. Stores the mixer's IP when it answers
 * @return 0 in case of success, -1 else
 */
int find_mixer();

/**
 * Sends a message to the mixer. Make sure to find the mixer first
 * @param message The message to send
 * @param len The message's length
 * @return the amount of bytes sent
 */
int to_mixer(char* message,int len);

/**
 * Listens for a message from the mixer and stores the this message
 * @return a net_ans package containung the message itself and its length
 */
net_ans* from_mixer();

#endif
