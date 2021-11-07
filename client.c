#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

char* host;
int port;
struct sockaddr_in remote_address;
int my_socket;
char* servername;

void *connection_handler(void *socket_desc);

void *connection_handler(void *socket_desc){
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
            /*read also some stuff - ja var nolasīt, tad var izprintet*/
        }
    }

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
    char* found;
    for (i =0; i< 2; i++) host = strsep(&randomhost,"=");
    for (i =0; i< 2; i++) realport = strsep(&ranodmport,"=");
    port = atoi(realport);

    printf("this is host :%s and this is port:%i      ", host, port);


    my_socket = 0;


    char inputs[100];
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);
    servername= gethostbyname(host);
    inet_pton(AF_INET, servername, &remote_address.sin_addr);

    if((my_socket = socket(AF_INET,SOCK_STREAM,0)) < 0){
        printf("SOCKET ERROR\n");
        return -1;
    }
    /* threading*/

    int *socketname = malloc(sizeof(int));
    *socketname = my_socket;

    pthread_t tred;
    pthread_create(&tred, NULL, connection_handler, socketname);
    pthread_join(tred, 0);

    return 0;
}
