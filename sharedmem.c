#include <errno.h>
#include <sys/mman.h>
#include "hw1.h"
#include <sys/types.h>
#include <stdio.h>

#define MAX_CLIENTS 10
#define PORT 12336
#define SHARED_MEMORY_SIZE 500

struct Packet* shared_memory = NULL;
void* memory_chunk = NULL;

struct Data {
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
    int windowWidth;
    int windowHeight;
    int score;

    int gameType;
    float playerX1;
    float playerY1;
    int playerHeight1;
    int playerWidth1;
    int playerColor1;

    float ballX;
    float ballY;
    int ballRadius;
    int ballColor;
    int powerUpCount;

/*game state server*/
    int upKey;
    int downKey;
    int leftKey;
    int rightKey;


/*check status ik pec 5 sec*/
/*game end server*/
    int gameDuration;
};

struct Packet {
    int PN;
    int packetID;
    long packetSize;
    char checkSum;
    struct Data;
    char seperator[2];
};

void get_shared_memory(){
    if(shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)){
        memory_chunk = (void*)shared_memory;
        memory_chunk = (memory_chunk + SHARED_MEMORY_SIZE - 1 - MAX_CLIENTS);
        printf("succesfully created buffer\n");
        return 0;
    }else{
        printf("could not mmap allocate MAXSIZE\n");
        return -1;
    }

}

int main(){
    get_shared_memory();
}