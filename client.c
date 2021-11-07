#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "hw1.h"

char* host;
char* port;

void sender(){

}

char* inputCheck(char*  haystack, char* needle ){
    char* result;
    int i = 0;
    char* ret = strstr(haystack, needle);
    /*printf("this is ret %s", ret);*/
    if (strcmp(ret, "-a=") >0 || strcmp(ret, "-p=") >0){
        for (i = 2; i< strlen(haystack); i++){
            result[i-2] = haystack[i];
        }
        return result;
    }
}

int main(int argc, char ** argv){

    if (argc < 3){
        printf("not enough arguments \n");
        return -1;
    }
    if (strcmp("client", argv[1]) < 0){
        printf("wrong input, client not called\n");
    }
    host = inputCheck(argv[2], "-a=");
    port = inputCheck(argv[3], "-p=");

    printf("this is host :%s and this is port:%s", host, port);


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
    /* threading*/

    /*
     if(connect(my_socket,(struct  sockaddr *) &remote_address,sizeof(remote_address)) < 0)
    {
        printf("ERROR connecting\n");
        return -1;
    }else{
        printf("good connection\n");
        while(1){
            char inputs[256];
            scanf("%s", inputs);
            //strcat("\n", inputs);
            send(my_socket, inputs, strlen(inputs), 0);
        }
    }
    */

    return 0;
}
