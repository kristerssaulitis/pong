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
#include <ctype.h>

#define MAXSIZE 1024

char* host;
int port;
struct sockaddr_in remote_address;


         /*global variables*/
/*----------------------------------*/
/*for client to understand at which state it is*/
int state = 0;
unsigned int PN = 8766;
/*----------------------------------*/

void *connection_handler(void* socket_desc);
void addSep( char * buf);
void addInt(int num,  char * buf);
void addLong(long num,  char * buf);
void add_string(char* str,  char* buf, int count);
char checksum(int length, char* packet);
int makeJoin( char* pointer, unsigned char* Username);
int makePlayerInput(char* pointer, char input);


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

        /*for sending from client to server*/
            int payload_size = 0;
            /*scanf("%s",inputs);*/     /*input from terminal for tests*/
            /*payload_size = makeJoin(inputs, "qwertyuiopasdfghjklz");*/
            /*payload_size = makeGameType(inputs, "1");*/
            /*payload_size = makeCheckStatus(inputs);*/
            /*payload_size = makePlayerReady(inputs);*/

            payload_size = makePlayerInput(inputs,'2');
            print_bytes(inputs, payload_size);
            send(my_sock,inputs, payload_size,0);
            memset(inputs, 0, payload_size);
            sleep(1);
            /*printf("hey, yolo, nemiz %s", inputs);*/

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


/*Note although I coppied here 10 functions the actual count for client is less some functions are used by client but some by server so it will change*/

int makeJoin(char* pointer, unsigned char* Username){
    char* buf = pointer;

    addSep(buf);
    addInt(PN, (char*)buf+2);
    addChar(&buf[6], '1'); /* packetID*/
    addInt(20, &buf[7]); /*packet size*/
    add_string(Username, &buf[11], strlen(Username));
    char checkSum_Char = checksum(29, &buf[2]);
    addChar(&buf[31], checkSum_Char);
    addSep(&buf[32]);
    return 34;
}



int makeMessage(char* pointer, char* message, char playerID){
    char* buf = pointer;

    addSep(buf); /*2*/
    addInt(PN, (char*)buf+2); /*6*/
    addChar(&buf[6], '3'); /* packetID 7*/
    addInt(258, &buf[7]); /*packet size 11*/
    addChar(&buf[11], playerID); /*12*/
    addChar(&buf[12], playerID); /*13*/
    add_string(message, &buf[13], strlen(message)); /*256+13 = 269*/
    char checkSum_Char = checksum(267, &buf[2]); /**/
    addChar(&buf[269], checkSum_Char); /*270*/
    addSep(&buf[270]); /*270*/
    return 272;
}

int makePlayerReady(char* pointer, char playerID){
    char* buf = pointer;

    addSep(buf); /*2*/
    addInt(PN, &buf[2]); /*6*/
    addChar(&buf[6], '3'); /* packetID 7*/
    addInt(1, &buf[7]); /*packet size 11*/
    addChar(&buf[11], playerID); /*12*/
    char checkSum_Char = checksum( 10 , &buf[2]);
    addChar(&buf[12], checkSum_Char); /*13*/
    addSep(&buf[13]);
    return 15;
}

int makePlayerInput(char* pointer, char input){
    char* buf = pointer;

    addSep(buf); /*2*/
    addInt(PN, (char*)buf+2); /*2+4=6*/
    addChar(&buf[6], '8'); /* packetID 6+1=7*/
    addInt(1, &buf[7]); /*packet size 7+4=11*/
    addChar(&buf[11], input); /*11+1=12*/
    char checkSum_Char = checksum( 10 , &buf[2]);
    addChar(&buf[12], checkSum_Char); /*12+1=13*/
    addSep(&buf[13]);/*13+2=15*/
    return 15;
}

int makeCheckStatus (char* pointer){
    char* buf = pointer;

    addSep(buf); /*2*/
    addInt(PN, (char*)buf+2); /*6*/
    addChar(&buf[6], '9'); /* packetID 7*/
    addInt(0, &buf[7]); /*packet size 11*/
    char checkSum_Char = checksum( 9 , &buf[2]);
    addChar(&buf[11], checkSum_Char); /*13*/
    addSep(&buf[12]);
    return 14;
}




char printable_char(char c){
    if(isprint(c) != 0 ) return c;
    return ' ';
}

void print_bytes(void* packet, int count){
    int i;
    char *p = (char*) packet;
    if(count > 999){
        printf("Cannot print more than 999 chars\n");
        return;
    }
    printf("printing %d bytes... \n", count);
    printf("[NPK] [C] [HEX] [DEC] [BINARY]\n");
    printf("===============================\n");
    for (i = 0; i< count; i++){
        printf("%3d | %c | %02X | %3d | %c%c%c%c%c%c%c%c\n", i , printable_char(p[i]), p[i], p[i],
        p[i] & 0x80 ? '1' : '0',
        p[i] & 0x40 ? '1' : '0',
        p[i] & 0x20 ? '1' : '0',
        p[i] & 0x10 ? '1' : '0',
        p[i] & 0x08 ? '1' : '0',
        p[i] & 0x04 ? '1' : '0',
        p[i] & 0x02 ? '1' : '0',
        p[i] & 0x01 ? '1' : '0'
        );
    }
}

/*packet asembly helper functions*/

void addSep( char * buf){
    buf[0] = '-';
    buf[1] = '-';
}

void addChar( char * buf, unsigned char ch){
    buf[0] = ch;
}

void fromInttoByte(int value, char* location){
    int i = 0;
    int s = sizeof(int);
    char*p  = location;
    for(i = 0;  i < s ; i++){
        p[i] = (value >> (s-i-1)*8) & 0xff;
    }
}

void addInt(int num, char * buf){
    int i = 0;
    char *p = buf;
    for(i = 0; i < sizeof(int); i++){
        p[i] = (num >> (sizeof(int)-1-i)*8) & 0xff;
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
    int i;
    char res=0;
    for(i = 0; i<length; i++){
        res ^= packet[i];
    }
    return res;
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
