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

    /*packet helper functions*/
char checksum(int length, char* packet){
    char checksum = 0;
    for(int i=0; i<length; i++){
        checksum ^= packet[i];
    }
    return checksum;
}

    /*Conection*/
void start_network(){
    int main_socket;
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

    if(bind(main_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0){
        printf("Error binding the main server socket!\n");
        exit(1);
    }printf("Main socket binded!\n");

    if(listen(main_socket, MAX_CLIENTS) < 0){
        printf("error listening to socket!\n");
        exit (1);
    }
    printf("Main socket is listening");

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

void process_client(int id, int socket){
    char in[1];
    char out[1000];

    printf("Process client id = %d, socket= %d\n", id, socket);
    printf("Client count %d\n", *client_count);

    /*maybe here start the thread for listener?*/
/*
    while(1){
        read(socket, in, 1);
        if ((int)in[0] > 13){
            int i = 0;
            char temp_str[256];
            shared_clients[MAX_CLIENTS + id] += 1;
            for(i = 0; i < (shared_data[MAX_CLIENTS + id]); i++){
                    temp_str[i] = (char)in[0];
                    */
                    /*printf("string in making -> %s\n", temp_str);*/
                    /*
                }
            temp_str[shared_data[MAX_CLIENTS + id]] = '\0';
            sprintf(out,"%s", temp_str);
            write(socket,out,strlen(out));
            printf("CLIENT %d read char %i\n", id , (int)in[0]);
        }
    }
*/
}


/*
void listener(int id, int socket){
    char in[256];
    char out[1000];
    char c;
    int sepCounter = 0;

    while(c=read()){
        if(c=='-'){
            c=read();
            if(c=='-'){
                sepCounter++;
                /*DO new packet */
/*
                if (sepCounter==2){
                    /* end the current one;*/
/*
                }
            } else{
                c = '-';
            }
        }
        if(c=='?'){
            c=read();
            if(c=='-') c = '-';
            else if(c=='*') c = '?';
        }
        else {
            /*read read read read read */
/*
        }
    }

}
*/
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
