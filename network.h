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

int s;
char* port= "10024";
char* BC="255.255.255.255";
char* IP="";
struct addrinfo hints;
struct addrinfo *servinfo;  // will point to the results
struct timeval to;
struct addrinfo *mixer_IP;
struct sockaddr_storage their_addr;
socklen_t addr_size = sizeof(their_addr);


void *get_addr(struct sockaddr *address)
{
    if (address->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)address)->sin_addr);
    }
    else {
        return &(((struct sockaddr_in6 *) address)->sin6_addr);
    }
}

void ini_bc_net(){
    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_INET;     
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_flags = AI_PASSIVE;     
    int broadcastEnable=1;
    getaddrinfo(BC, port, &hints, &servinfo);
    s=socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    if (setsockopt (s, SOL_SOCKET, SO_RCVTIMEO, (char *)&to,
                sizeof(to)) < 0)
        printf("setsockopt failed\n");

}

int find_mixer(){

    char* infomessage=calloc(8,sizeof(char));
    sprintf(infomessage,"/xinfo%c",0);
    sendto(s,infomessage,7,0,servinfo->ai_addr,servinfo->ai_addrlen);
    free(infomessage);
    char* answerbuffer=calloc(128,sizeof(char));
    int received_bytes=recvfrom(s,answerbuffer,127,0,(struct sockaddr *)&their_addr, &addr_size);
    printf("ich suche nun den Mixer!\n");
    char sock_address[46];
    printf("ich habe antwort: %d bytes\n",received_bytes);
    if (received_bytes!=-1){
        char* temp=(char*)inet_ntop(their_addr.ss_family,
                     get_addr((struct sockaddr *)&their_addr),
                     sock_address, sizeof sock_address);
        printf("IP des Mixers: %s\n",temp);
        getaddrinfo(temp, port, &hints, &mixer_IP);
        free(answerbuffer);
        return 0;
    }
    free(answerbuffer);
    return -1;
}

int to_mixer(char* message,int len){
    int bytes_sent=sendto(s,message,len,0,mixer_IP->ai_addr,mixer_IP->ai_addrlen);
    return bytes_sent;
}

net_ans* from_mixer(){
    net_ans* answer=calloc(1,sizeof(net_ans));
    answer->msg=calloc(128, sizeof(char));
    answer->len=recvfrom(s,answer->msg,127,0,(struct sockaddr *)&their_addr, &addr_size);
    return answer;
}

#endif
