#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>

#define MAX_CLIENTS 4
#define PORT 12336

void* shared_memory = NULL;
struct Ball* shared_balls = NULL;
struct Client* shared_clients= NULL;

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

struct Ball baller;

int PN;
int packetID;
long packetSize;
char checkSum;
    /* char seperator[2]; */

void get_shared_memory(){
    int sizeofthings = sizeof(struct Ball)+ MAX_CLIENTS*sizeof(struct Client);
    if(shared_memory = mmap(NULL, sizeofthings, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)){
        shared_balls = (struct Ball*) shared_memory;
        shared_clients = (struct Client*) (sizeof(shared_balls) + shared_memory);

        printf("succesfully created buffer - balls and\n");
        return 0;
    }else{
        printf("could not mmap allocate MAXSIZE\n");
        return -1;
    }

}

int main(){
    get_shared_memory();
}



void listener(int id, int socket){
    char in[1];
    char out[1000];
    char c;
    int sepCounter = 0;

    while(c=read()){
        if(c=='-'){
            c=read();
            if(c=='-'){
                sepCounter++;
                /*DO new packet */
                if (sepCounter==2){
                    /* end the current one;*/
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

        }
    }

}
