#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>

#define MAXSIZE 1024

char* host;
int port;
struct sockaddr_in remote_address;

void *connection_handler(void* socket_desc);
void addSep( char * buf);
void addInt(int num,  char * buf);
void addLong(long num,  char * buf);
void add_string(char* str,  char* buf, int count);
char checksum(int length, char* packet);
 char * makePacket1( char* pointer, char* Username);


void *connection_handler(void* args){
    char inputs[256];
    int my_sock = *(int*) args;
    free(args);

    if(connect(my_sock,(struct sockaddr *) &remote_address,sizeof(remote_address)) < 0){
        printf("ERROR connecting\n");
        exit(1);
    }else{
        char buffer [MAXSIZE];
        memset(buffer, 0, MAXSIZE);
        printf("good connection\n");
        while(1){
            strcpy(buffer, "");
            /*scanf("%s",inputs);*/
            makePacket1(inputs, "ndfvna");
            
            send(my_sock,inputs,strlen(inputs),0);
            /*printf("hey, yolo, nemiz %s", inputs);*/
            fflush(stdout);
            memset(inputs, 0, 256);
            /*
            sleep(1);
            if (read(my_sock, buffer, MAXSIZE-1)>0) printf("buffer: %s", buffer);
            fflush(stdout);
            */
        }
    }
    return NULL;
}




/*Packet functions*/


 char * makePacket1( char* pointer,char* Username){
    int PN = 1;
     char* buf = pointer;

    addSep(buf);
    printf("yolo, 2.0 %s      ", buf);
    addInt(PN, &buf[2]);
    printf("yolo, 3.0 %s      ", buf);
    addInt(1, &buf[6]);
    printf("yolo, 4.0 %s      ", buf);
    addLong(20, &buf[10]);
    printf("yolo, 5.0 %s      ", buf);
    char checkSum_Char = checksum( 20, Username);
    printf("yolo, 6.0 %s      ", buf);
    add_string(&checkSum_Char, &buf[18], 1);
    printf("yolo, 7.0 %s      ", buf);
    add_string(Username, &buf[19], strlen(Username));
    printf("yolo, 8.0 %s      ", buf);
    addSep(&buf[39]);
    printf("yolo, 9.0 %s      ", buf);
    buf[41] = '\0';
    printf("yolo, 10.0 %s      ", buf);
    return buf;
}







/*packet asembly helper functions*/

void addSep( char * buf){
    buf[0] = '-';
    buf[1] = '-';
}

void addInt(int num,  char * buf){
    int i = 0;
    for(i = 0; i < sizeof(int); i++){
        buf[i] = (num >> (sizeof(int)-1-i)*8) & 0xff;
    }
}

void addLong(long num,  char * buf){
    int i = 0;
    buf[0] = num & 0xff;
    for(i = 0; i < sizeof(long); i++){
        buf[i] = (num >> (sizeof(long)-1-i)*8) & 0xff;
    }
}

void add_string(char* str,  char* buf, int count){
    int i = 0;
    for(i = 0; i < count; i++){
        buf[i] = str[i];
    }
}

char checksum(int length, char* packet){
    char checksum = 0;
    for(int i=0; i<length; i++){
        checksum ^= packet[i];
    }
    return checksum;
}
/*______________________________________________________________________________________-*/




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

    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);
    servername = gethostbyname(host);

    inet_pton(AF_INET, servername, &remote_address.sin_addr);
        /*
        printf("tiek līdz šejienei");
        fflush(stdout);
        */
    if((my_socket = socket(AF_INET,SOCK_STREAM,0)) < 0){
        printf("SOCKET ERROR\n");
        return -1;
    }

    int *socketname = malloc(sizeof(int));
    *socketname = my_socket;
    /*
    printf("tiek līdz šejienei");
    fflush(stdout);
    */
    pthread_t tred;
    pthread_create(&tred, NULL, connection_handler, socketname);
    pthread_join(tred, 0);

    return 0;
}
