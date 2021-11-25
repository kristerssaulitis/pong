#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <ctype.h>

/*global constants and varieables*/
#define MAX_CLIENTS 4
#define PORT 12336
int port;


/*Shared memory pointers*/
void* shared_memory = NULL;
int* client_count = NULL;       /*client count starts from 0 so to get client count <- client_count + 1*/
struct Ball* shared_balls = NULL;
struct Client* shared_clients= NULL;

/*Shared memory structures*/ /*We might need to reset some values to default because they get initialized with some trash*/
struct Client {
    /*join client*/
    char name[20];
    /*lobby server*/
    int status;
    char error[100];
    /*game type client*/
    int type;
    /*player queue server ari int status*/
    /*game server ari int status*/
    /*player ready = empty*/
    /*game state server*/

    int score;

    int gameType;
    float playerX1;
    float playerY1;
    int playerHeight1;
    int playerWidth1;
    int playerColor1;


/*game state server*/
    int upKey;
    int downKey;
    int leftKey;
    int rightKey;

/*check status ik pec 5 sec*/
/*game end server*/
    int gameDuration;
};
struct Ball {
    float ballX;
    float ballY;
    int ballRadius;
    int ballColor;
    int powerUpCount;
    int windowWidth;
    int windowHeight;
};






/*Networking functions*/
/*_____________________________________________________________________________________________________________*/

    /*shared memory*/
void get_shared_memory(){
    int sizeofthings = sizeof(struct Ball)+ MAX_CLIENTS*sizeof(struct Client) + sizeof(int);
    if(shared_memory = mmap(NULL, sizeofthings, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)){
        client_count = shared_memory;
        shared_balls = (struct Ball*) shared_memory + sizeof(int);
        shared_clients = (struct Client*) (sizeof(shared_balls) + shared_memory);

        /*initializing objects*/
        *client_count = 0;     /*NOT SURE ABOUT THE ORDER -1 or 0*/
        struct Ball gameball = *shared_balls;
        int c_iterator = 0;
        for (c_iterator; c_iterator < MAX_CLIENTS; c_iterator++){
            struct Client cl = shared_clients[c_iterator];
        }

        /*success & error messages*/
        printf("succesfully created buffer - balls and pong sticks\n");
        return 0;
    }else{
        printf("could not mmap allocate MAXSIZE\n");
        return -1;
    }

}

void direct_copy_data_as_bytes(void* packet, void* data, int size){
    /* Different results on different thusam machines, But can store any data type!*/
    int i;
    char* p = packet;
    char* d = data;
    for(i=0;i<size;i++){
        p[i] = d[i];
    }
}

    /*packet helper functions*/
char checksum(int length, char* packet){
    char checksum = 0;
    for(int i=0; i<length; i++){
        checksum ^= packet[i];
    }
    return checksum;
}

int is_little_endian_system(){
    volatile uint32_t i=0x01234567;
     return (*((uint8_t*)(&i))) == 0x67;
}
/* 1 = little, 0 = big*/

void universal_store_int_as_bytes_big_endian(void* packet, int data){
    int i;
    int s = sizeof(int);
    char* p = packet;
    for(i=0; i<s; i++){
        p[i] = (data >> (s-i-1)*8) & 0xFF;
    }
}

void universal_store_int_as_bytes_little_endian(void* packet, int data){
    int i;
    int s = sizeof(int);
    char* p = packet;
    for(i=0; i<s; i++){
        p[i] = (data >> (i*8)) & 0xFF;
    }
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
        printf("%3d | %c | %02X | %c%c%c%c%c%c%c%c\n", i , printable_char(p[i]), p[i], p[i],
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



    /*Conection*/
void start_network(){
    int main_socket;
    int opt_value = 1;
    struct sockaddr_in server_address;
    int client_socket;
    struct sockaddr_in client_address;
    int client_address_size = sizeof(client_address);

    main_socket = socket(AF_INET, SOCK_STREAM,0);
    if(main_socket < 0){printf("Error opening main server socket!\n"); exit(1);}
    printf("Main socket created!\n");

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, (const void*) &opt_value, sizeof(int));

    if(bind(main_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        printf("Error binding the main server socket!\n");
        exit(1);
    }printf("Main socket binded!\n");

    if(listen(main_socket, MAX_CLIENTS) < 0){
        printf("error listening to socket!\n");
        exit (1);
    }
    printf("Main socket is listening");
    /*fflush(stdout);*/
    while(1){
        int new_client_id = 0;
        int cpid = 0;

        client_socket = accept(main_socket, (struct socaddr*) &client_address, &client_address_size);
        if (client_socket < 0){
            printf("Error accepting client connection! ERRNO=%d\n", errno);
            continue;
        }

        new_client_id = *client_count;
        *client_count +=1;
        cpid = fork();

        if(cpid == 0){
            close(main_socket);
            cpid = fork();
            if(cpid == 0){

                /*fflush(stdout);*/
                process_client(new_client_id, client_socket);
                exit(0);
            }else{
                wait(NULL); /*might not need this line*/
                printf("Succesfully orphaned client %d\n", new_client_id);
                exit(0);
            }
        }else{
            close(client_socket);
        }
    }
}

int is_little_endian_system( ){
    volatile uint32_t i=0x01234567;
    return (*((uint8_t*)(&i)));
    /*1 = little, 0 = BIG*/
}


void unwrapping(char * out){
    int i = 0;
    printf("might work???%s", out);
    fflush(stdout);
/*endieness  -> if (is_little_endian_system()== 1){} else {}*/
if (is_little_endian_system()== 1){
    /*convert to big*/
    printf("small small endian");
}

/*
packet number
checksum
escaping
    if(c=='?'){
        c=read(socket, in, 1);
        if(c=='-') c = '-';
        else if(c=='*') c = '?';
    }
*/
}

void process_client(int id, int socket){
    char in[1];
    char out[1000];
    int sepCounter = 0;
    int inpacket = 1;
    /*0 = ja', 1 = nē*/
    int i = 0;
/*-- nfdnvakjnfvjkdnfbkanf -- */
/*šeit no iepriekšājā paliek -- abcdejdirst, šeit no nākamā sākas -- */
/*-- nfdnvakjnfvjkdnfbkanf -- */
    while(1){
        printf("gang gan gangangang");
        fflush(stdout);
        while(1){
            read(socket, in, 1);

            if (inpacket == 0){
                if(in[0]=='-'){
                    read(socket, in, 1);
                    if(in[0]=='-'){
                        ++sepCounter;
                        printf("\n do you even listen 5 .0 %c \n ", in[0]);
                            if (sepCounter==2){
                                sepCounter = 0;
                                inpacket = 1;
                                /*nfdnvakjnfvjkdnfbkanf  ->šis viss tagad ir out'ā*/
                                out[i] = '\0';
                                unwrapping(out);
                                memset(out, 0, 1000);
                                break;
                            }
                        } else {
                            out[i] = '-';
                            out[i+1] = in[0];
                            i++;
                        }
                    } else {
                        /*read read read read read */
                        printf("\n do you even listen daudaudzdua .0 %c \n ", in[0]);
                        out[i] = in[0];
                    }
                    i++;
                } else {
                    printf("\n do you even listen 2.0 %c \n ", in[0]);
                    if(in[0]=='-'){
                        read(socket, in, 1);
                        if(in[0]=='-'){
                            printf("\n do you even listen 4 .0 %c \n ", in[0]);
                            fflush(stdout);
                            ++sepCounter;
                            inpacket = 0;
                        }
                }
            }
        }
    }
    /*Can write to char out[] untill the end -- is detected when detected send it to unwraper (also empty out[] for next packet) and wait untill -- detected again to start next packet*/

}

/*   ^^   process client is used in start network and it is started as separet process for reading socket so just write what you would write in listener directly into process_client        */

/*_____________________________________________________________________________________________________________*/



/*Game logics*/
void gameloop(){
    printf("Started game loop! (it will run forever - use ctrl+C)\n");
    int i=0;
    /*currently does not do shit - but it does not have to*/
    while(1){
        for(i = 0; i<*client_count; i++){
            /*
            shared_clients[MAX_CLIENTS +i];
            shared_clients[i] = 0;
            */
        }
        sleep(1);
    }
}






/*Server main*/
int main(int argc, char** argv){

    int i = 0;
    if (strncmp("-p=", argv[1],3) < 0){
        printf("wrong parameter PORT %s \n", argv[2]);
        return -1;
    }
    char* realport;
    char *ranodmport = argv[1];
    for (i =0; i< 2; i++) realport = strsep(&ranodmport,"=");
    port = atoi(realport);
    printf("tiek līdz šejienei");
    fflush(stdout);
    int pid = 0;
    printf("SERVER started!\n");
    get_shared_memory();

    pid = fork();
    if(pid ==0){
        start_network();
    }else{
        gameloop();
    }

    return 0;
}
