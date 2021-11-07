#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAXSIZE 1024
#define PORT 12333
#define HOST "localhost"

char* host;
int port;
struct sockaddr_in remote_address;

void *connection_handler(void* socket_desc);

void* connecter(void* args){

}

void *connection_handler(void* args){
    char buffer [MAXSIZE];
    char output[]="client: Enter data for server: ";
    int r;
    return;
    printf("tiek līdz šejienei");
    fflush(stdout);
    if(connect(args,(struct sockaddr *) &remote_address,sizeof(remote_address)) < 0){
        printf("ERROR connecting\n");
        return -1;
    }else{
        printf("good connection\n");
        while(1){
            char inputs[256];
            write(1,output,strlen(output));
            r=read(0,buffer,sizeof(buffer));
            buffer[r-1]='\0';
            write(args,buffer,strlen(buffer));
            read(args, buffer, sizeof(buffer));
            printf("Answer from server: %s\n",buffer);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char ** argv){

    if (argc < 2){
        printf("not enough arguments \n");
        return -1;
    }

    int i = 0;
    if (strncmp("-a=", argv[1],3) < 0){
        printf("wrong parameter HOST");
        return -1;
    }
    if (strncmp("-p=", argv[2],3) < 0){
        printf("wrong parameter PORT %s \n", argv[2]);
        return -1;
    }

    char* realport;
    char *randomhost = argv[1];
    char *ranodmport = argv[2];

    for (i =0; i< 2; i++) host = strsep(&randomhost,"=");
    for (i =0; i< 2; i++) realport = strsep(&ranodmport,"=");
    port = atoi(realport);

    printf("this is host :%s and this is port:%i      ", host, port);


    int my_socket = 0;
    char* servername;

    remote_adress.sin_family = AF_INET;
    remote_adress.sin_port = htons(port);
    servername =gethostbyname(host);
    printf("tiek līdz šejienei");
    fflush(stdout);
    int retval = inet_pton(AF_INET, servername, &remote_address.sin_addr);
    if (retval == 0){
        printf("tiek līdz šejienei");
    fflush(stdout);
    }

    if((my_socket = socket(AF_INET,SOCK_STREAM,0)) <= 0){
        printf("SOCKET ERROR\n");
        return -1;
    }

    int *socketname = malloc(sizeof(int));
    *socketname = my_socket;

    pthread_t tred;
    pthread_create(&tred, NULL, connection_handler, socketname);
    pthread_join(tred, 0);

    return 0;
}
