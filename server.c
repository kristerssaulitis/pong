#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>

#define MAX_CLIENTS 10
#define PORT 12331
#define SHARED_MEMORY_SIZE 10

char* shared_memory = NULL;
int* client_count = NULL;
int* shared_data = NULL;

void get_shared_memory(){
    shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    client_count = (int*) shared_memory;
    shared_data = (int*) (shared_memory + sizeof(int));
}

void gameloop(){
    printf("Started game loop! (it will run forever - use ctrl+C)\n");
    int i=0;
    while(1){
        for(i = 0; i<*client_count; i++){
            shared_data[MAX_CLIENTS +i] += shared_data[i];
            shared_data[i] = 0;
        }
        sleep(1);
    }
}

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
    server_address.sin_port = htons(PORT);

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
    int i = 0;
    char in[1];
    char out[100];

    printf("Process client id = %d, socket= %d\n", id, socket);
    printf("Client count %d\n", *client_count);

    while(1){
        read(socket, in, 1);
        if(in[0] > 47 && in[0] <58){
            sprintf(out, "CLIENT %d SUM = %d\n", id, shared_data[MAX_CLIENTS + id]);
            write(socket,out,strlen(out));
            i = (int)in[0] -48;
            printf("CLIENT %d read number %d\n", id , i);
            /*printf("CLIENT %d read number %d\n", id , i);*/ /*was printing the input value - old version*/
            shared_data[id] = i;
        }
    }
}

int main(){
    int pid = 0;
    int i;
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
