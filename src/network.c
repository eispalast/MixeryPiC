#include "network.h"


char* port= "10024"; //the port that the X-Air 18 works on
char* BC="255.255.255.255";
char* IP="";

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
    char sock_address[46];
    if (received_bytes!=-1){
        char* temp=(char*)inet_ntop(their_addr.ss_family,
                     get_addr((struct sockaddr *)&their_addr),
                     sock_address, sizeof sock_address);
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

