#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "hw1.h"

#define HOST "localhost"
#define PORT 12331

char* inputCheck(char* argv){
    char* result;
    if (str_length(argv) == 6){

    }
}

int main(int argc, char ** argv){
    if (argc < 3){
        printf("not enough arguments \n");
        return -1;
    }
    char* host;
    char* port;
    host = inputCheck(argv[1]);
    port = inputCheck(argv[2]);


    int my_socket = 0;
    char* servername;
    struct sockaddr_in remote_address;
    char inputs[100];
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(PORT);
    servername= gethostbyname(HOST);
    inet_pton(AF_INET, servername, &remote_address.sin_addr);

    if((my_socket = socket(AF_INET,SOCK_STREAM,0)) < 0){
        printf("SOCKET ERROR\n");
        return -1;
    }
    // threading

    // if(connect(my_socket,(struct  sockaddr *) &remote_address,sizeof(remote_address)) < 0)
    // {
    //     printf("ERROR connecting\n");
    //     return -1;
    // }else{
    //     printf("good connection\n");
    //     while(1){
    //         char inputs[256];
    //         scanf("%s", inputs);
    //         //strcat("\n", inputs);
    //         send(my_socket, inputs, strlen(inputs), 0);
    //     }
    // }

    return 0;
}
