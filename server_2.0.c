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
void *shared_memory = NULL;
int *client_count = NULL; /*client count starts from 0 so to get client count <- client_count + 1 <- might not be true fake news*/
struct Ball *shared_balls = NULL;
struct Client *shared_clients = NULL;
char* out_packets = NULL;
struct OutBuffer *shared_buffer = NULL;

char checksum(int length, char *packet);

/*Shared memory structures*/ /*We might need to reset some values to default because they get initialized with some trash*/

struct OutBuffer{
    int payload;
    char output[200];
};

struct Client
{
    int PN;
    int PNC;
    char name[21];
    char playerID;
    char targetID;
    char gameType;
    char teamID;
    char ready;
    char status;

    int socket;
    /*player queue server ari int status*/
    /*game server ari int status*/
    /*player ready = empty*/
    /*game state server*/
    int gameStatus;


    int scoreTeam;
    int score;
    float playerX1; float playerY1;
    int playerHeight1; int playerWidth1;
    int playerColor1;

    /*game state server*/
    char upKey;
    char downKey;
    char leftKey;
    char rightKey;
    char exit;
    char action;
    float team_goal1X; float team_goal1Y;
    float team_goal2X; float team_goal2Y;

    /*check status ik pec 5 sec*/
    /*game end server*/
    int gameDuration;
};
struct Ball
{
    float ballX; float ballY;
    int ballRadius; int ballColor;
    char ballCount;
    char teamCount; char playerCount;
    int powerUpCount; char powerUpType;
    int windowWidth; int windowHeight;
    int gameDuration;
    int ID1;
    float powerUpX1; float powerUpY1;
    int powerUpWidth1; int powerUpHeight1;
};


/*Networking functions*/
/*_____________________________________________________________________________________________________________*/


/*packet functions*/
/*_____________________________________________________________________________________________________________*/

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
    char res = 0;
    for(i = 0; i<length; i++){
        res ^= packet[i];
    }
    return res;
}

int makeAccept(char* pointer, int id){
    char* buf = pointer;
    int PN = shared_clients[id].PNC;
    char playerID = shared_clients[id].playerID;

    int ret = 0;

    addSep(buf); /*2*/ ret += 2;
    addInt(PN, &buf[ret]); /*6*/ ret += 4;
    addChar(&buf[ret], '2'); /* packetID 7*/ ret += 1;
    addInt(1, &buf[ret]); /*packet size 11*/ ret += 4;
    addChar(&buf[ret], playerID); /*12*/ ret += 1;
    char checkSum_Char = checksum( ret-2 , &buf[2]);
    addChar(&buf[ret], checkSum_Char); /*13*/ ret+=1;
    addSep(&buf[ret]); ret += 2;
    return ret;
}

int makeMessage(char* pointer, char* message, int id){
    char* buf = pointer;
    int PN = shared_clients[id].PNC;
    char targetID = shared_clients[id].targetID;
    char sourceID = 'a';
    int ret = 0;
    /*[0=chat, 1=info, 2=error]*/
    addSep(buf); /*2*/ ret += 2;
    addInt(PN, (char*)buf+ret); /*6*/ ret += 4;
    addChar(&buf[ret], '3'); /* packetID 7*/ ret += 1;
    addInt(258, &buf[ret]); /*packet size 11*/ ret += 4;
    addChar(&buf[ret], targetID); /*12*/ ret += 1;
    addChar(&buf[ret], sourceID); /*13*/ ret += 1;
    add_string(message, &buf[ret], 256); /*256+13 = 269*/ ret += 256;
    char checkSum_Char = checksum(ret-2, &buf[2]); /**/
    addChar(&buf[ret], checkSum_Char); /*270*/  ret += 1;
    addSep(&buf[ret]); /*270*/ ret += 2;
    return ret;
}

int makeLobby(char* pointer, int id){
    char* buf = pointer;
    int PN = shared_clients[id].PNC;
    char name[20];
    strcpy(name,shared_clients[id].name);
    char count = shared_balls->playerCount;
    char playerID = shared_clients[id].playerID;
    char playerCou = shared_balls->playerCount;
    int playerCount = playerCou - '0';
    int i = 0;
    int ret = 0;
    addSep(buf); /*2*/ ret += 2;
    addInt(PN, &buf[ret]); /*6*/ ret += 4;
    addChar(&buf[ret], '4'); /* packetID 7*/ ret += 1;
    addInt(1+ 21*(playerCount), &buf[ret]); ret += 4;
/*here will be the apaksa aizkomentetais kods*/
    addChar(&buf[ret], count); /*12*/ ret += 1;
    for (i; i < (playerCount); i++){
        addChar(&buf[ret], shared_clients[i].playerID); ret += 1;
        add_string(shared_clients[i].name, &buf[ret], 20); ret += 20;
    }
    char checkSum_Char = checksum( ret-2, &buf[2]);
    addChar(&buf[ret], checkSum_Char); /*13*/ ret += 1;
    addSep(&buf[ret]); ret += 2;
    return ret;

}

int makeGameReady( char* pointer, int id ){
    char* buf = pointer;
    int i = 0;
    int PN = shared_clients[id].PNC;
    int windowWidth = shared_balls->windowWidth;
    int windowHeight = shared_balls->windowHeight;
    char teamCou = shared_balls->teamCount;
    int teamCount = teamCou - '0';
    char teamID = shared_clients[id].teamID;
    char playerCou = shared_balls->playerCount;
    int playerCount = playerCou - '0';
    int ret = 0;

    addSep(buf); ret += 2;
    addInt(PN, &buf[ret]); /*6*/ ret += 4;
    addChar(&buf[ret], '5'); /* packetID 7*/ ret += 1;
    addInt(10+17*(teamCount) + 39*(playerCount), &buf[ret]); /*packet size 11*/ ret += 4;
    addInt(windowWidth, &buf[ret]); ret += 4;
    addInt(windowHeight, &buf[ret]); ret += 4;
    addChar(&buf[ret], teamCou); ret += 1;
    for (i; i < (teamCount); i++){
        addChar(&buf[ret], shared_clients[i].teamID); ret += 1;
        addInt(shared_clients[i].team_goal1X, &buf[ret]); ret += 4;
        addInt(shared_clients[i].team_goal1Y, &buf[ret]); ret += 4;
        addInt(shared_clients[i].team_goal2X, &buf[ret]); ret += 4;
        addInt(shared_clients[i].team_goal2Y, &buf[ret]); ret += 4;
    }
    addChar(&buf[ret], playerCou);
    for (i; i < (playerCount); i++){
        addChar(&buf[ret], shared_clients[i].playerID); ret += 1;
        addChar(&buf[ret], shared_clients[i].ready); ret += 1;
        addChar(&buf[ret], shared_clients[i].teamID); ret += 1;
        add_string(shared_clients[i].name, &buf[ret], 20); ret += 10;
        addInt(shared_clients[i].playerX1, &buf[ret]); ret += 4;
        addInt(shared_clients[i].playerY1, &buf[ret]); ret += 4;
        addInt(shared_clients[i].playerWidth1, &buf[ret]); ret += 4;
        addInt(shared_clients[i].playerHeight1, &buf[ret]); ret += 4;
    }
    char checkSum_Char = checksum(ret-2, &buf[2]);
    addChar(&buf[ret], checkSum_Char); /*13*/ ret += 1;
    addSep(&buf[ret]); ret += 2;
    return ret;
}

int makeGameState( char* pointer, int id ){
    char* buf = pointer;
    int i = 0;
    int ret = 0;

    int PN = shared_clients[id].PNC;
    int windowWidth = shared_balls->windowWidth;
    int windowHeight = shared_balls->windowHeight;
    char playerCou = shared_balls->playerCount;
    int playerCount = playerCou - '0';
    char ballCou = shared_balls->ballCount;
    int ballCount = ballCou - '0';
    char powerUpCou = shared_balls->powerUpCount;
    int powerUpCount = powerUpCou - '0';
    char teamCou = shared_balls->teamCount;
    int teamCount = teamCou - '0';

    addSep(buf); ret += 2;
    addInt(PN, &buf[ret]); /*6*/ ret += 4;
    addChar(&buf[ret], '7'); /* packetID 7*/ ret += 1;
    addInt(12+21*(teamCount) + 18*(playerCount) + 13*(ballCount) + 17*(powerUpCount), &buf[ret]); /*packet size 11*/ ret += 4;
/*start the data seg*/
    addInt(windowWidth, &buf[ret]); ret += 4;
    addInt(windowHeight, &buf[ret]); ret += 4;
    addChar(&buf[ret], teamCou); ret += 1;
    for (i; i < (teamCount); i++){
        addChar(&buf[ret], shared_clients[i].teamID); ret += 1;
        addInt(shared_clients[i].scoreTeam, &buf[ret]); ret += 4;
        addInt(shared_clients[i].team_goal1X, &buf[ret]); ret += 4;
        addInt(shared_clients[i].team_goal1Y, &buf[ret]); ret += 4;
        addInt(shared_clients[i].team_goal2X, &buf[ret]); ret += 4;
        addInt(shared_clients[i].team_goal2Y, &buf[ret]); ret += 4;
    }
    addChar(&buf[63], playerCou); ret += 1;
    for (i = 0; i < (playerCount); i++){
        addChar(&buf[ret], shared_clients[i].playerID); ret += 1;
        addChar(&buf[ret], shared_clients[i].teamID); ret += 1;
        addInt(shared_clients[i].playerX1, &buf[ret]); ret += 4;
        addInt(shared_clients[i].playerY1, &buf[ret]); ret += 4;
        addInt(shared_clients[i].playerWidth1, &buf[ret]); ret += 4;
        addInt(shared_clients[i].playerHeight1, &buf[ret]); ret += 4;
    }
    addChar(&buf[ret], ballCou); ret += 1;
    for (i = 0; i < (ballCount); i++){
        addInt(shared_balls->ballX, &buf[ret]); ret += 4;
        addInt(shared_balls->ballY, &buf[ret]); ret += 4;
        addInt(shared_balls->ballRadius, &buf[ret]); ret += 4;
        addChar(&buf[ret], shared_balls->ballColor); ret += 1;
    }
    addChar(&buf[ret], powerUpCou); ret += 1;
    for (i = 0; i < (powerUpCount); i++){
        addChar(&buf[ret], shared_balls->powerUpType); ret += 1;
        addInt(shared_balls->powerUpX1, &buf[ret]); ret += 4;
        addInt(shared_balls->powerUpY1, &buf[ret]); ret += 4;
        addInt(shared_balls->powerUpWidth1, &buf[ret]); ret += 4;
        addInt(shared_balls->powerUpHeight1, &buf[ret]); ret += 4;
    }
    /*end the data seg*/
    char checkSum_Char = checksum(ret-2, &buf[2]);
    addChar(&buf[ret], checkSum_Char); /*13*/ ret += 1;
    addSep(&buf[ret]); ret += 2;
    return ret;
}

int makeGameEnd (char* pointer, int id, char status ){
    char* buf = pointer;
    int ret = 0;
    int i = 0;

    int PN = shared_clients[id].PNC;
    int scoreTeam = shared_clients[id].scoreTeam;
    int gameDuration = shared_balls->gameDuration;
    char teamCou = shared_balls->teamCount;
    int teamCount = teamCou - '0';
    char teamID = shared_clients[id].teamID;
    char playerCou = shared_balls->playerCount;
    int playerCount = playerCou - '0';
    char playerID = shared_clients[id].playerID;
    char name[20];
    strcpy(name,shared_clients[id].name);
    int score = shared_clients[id].score;

    addSep(buf); ret += 2;
    addInt(PN, &buf[ret]); /*6*/ ret += 4;
    addChar(&buf[ret], '10'); /* packetID 7*/ ret += 1;
    addInt(11+ (teamCount)*5 + (playerCount)*26, &buf[ret]); /*packet size 11*/ ret += 4;
/*start the data seg*/
    addChar(&buf[ret], status); ret += 1;
    addInt(scoreTeam, &buf[ret]); ret += 4;
    addInt(gameDuration, &buf[ret]); ret += 4;
    addChar(&buf[ret], teamCou); ret += 1;
    for (i = 0; i < (teamCount); i++){
        addChar(&buf[ret], shared_clients[i].teamID); ret += 1;
        addInt(shared_clients[i].scoreTeam, &buf[ret]); ret += 4;
    }
    addChar(&buf[ret], playerCou); ret += 1;
    for (i = 0; i < (playerCount); i++){
        addChar(&buf[ret], shared_clients[i].playerID); ret += 1;
        addChar(&buf[ret], shared_clients[i].teamID); ret += 1;
        addInt(shared_clients[i].score, &buf[ret]); ret += 4;
        add_string(shared_clients[i].name, &buf[ret], 20); ret += 20;
    }
    /*end the data seg*/
    char checkSum_Char = checksum(ret-2, &buf[2]);
    addChar(&buf[ret], checkSum_Char); /*13*/ ret += 1;
    addSep(&buf[ret]); ret += 2;
    return ret;
}
/*pretty functions*/
/*_____________________________________________________________________________________________________________*/
/*shared memory*/
void get_shared_memory()
{
    int sizeofthings = sizeof(struct Ball) + MAX_CLIENTS * sizeof(struct Client) + sizeof(int) + MAX_CLIENTS * sizeof(struct OutBuffer) + 100;
    if (shared_memory = mmap(NULL, sizeofthings, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))
    {
        client_count = shared_memory;
        shared_balls = (struct Ball *)shared_memory + sizeof(int);
        shared_clients = (struct Client *)(sizeof(shared_balls) + shared_balls);
        shared_buffer = (struct OutBuffer *)(shared_clients + MAX_CLIENTS * sizeof(shared_clients));

        /*initializing objects*/
        *client_count = 0; /*NOT SURE ABOUT THE ORDER -1 or 0*/
        struct Ball gameball = *shared_balls;
        int b_punkts = 0;
        int c_iterator = 0;

        printf("PILNIGS PIZDJUKS sizeof(struct Client) %i un otrs sizeof(shared_clients) %i\n", sizeof(struct Client), sizeof(shared_clients));
        fflush(stdout);

        for (c_iterator; c_iterator < MAX_CLIENTS; c_iterator++){
            struct Client cl = shared_clients[c_iterator];
            struct OutBuffer buf = shared_buffer[c_iterator];
            buf.payload = 0;
            cl.PN = 0;
            cl.PNC = 0;
            cl.status = '0'; /*not ready*/
            cl.gameStatus = 0;
        }
    /*
        for (b_punkts; b_punkts < MAX_CLIENTS; b_punkts++){
            struct OutBuffer buf = shared_buffer[b_punkts];
            buf.payload = 0;
        }
    */

        /*success & error messages*/
        printf("succesfully created buffer - balls and pong sticks\n");
        return 0;
    }
    else
    {
        printf("could not mmap allocate MAXSIZE\n");
        return -1;
    }
}

void direct_copy_data_as_bytes(void *packet, void *data, int size)
{
    /* Different results on different thusam machines, But can store any data type!*/
    int i;
    char *p = packet;
    char *d = data;
    for (i = 0; i < size; i++)
    {
        p[i] = d[i];
    }
}

int is_little_endian_system()
{
    volatile uint32_t i = 0x01234567;
    return (*((uint8_t *)(&i))) == 0x67;
}
/* 1 = little, 0 = big*/

void universal_store_int_as_bytes_big_endian(void *packet, int data)
{
    int i;
    int s = sizeof(int);
    char *p = packet;
    for (i = 0; i < s; i++)
    {
        p[i] = (data >> (s - i - 1) * 8) & 0xFF;
    }
}

void universal_store_int_as_bytes_little_endian(void *packet, int data)
{
    int i;
    int s = sizeof(int);
    char *p = packet;
    for (i = 0; i < s; i++)
    {
        p[i] = (data >> (i * 8)) & 0xFF;
    }
}

char printable_char(char c)
{
    if (isprint(c) != 0)
        return c;
    return ' ';
}

void print_bytes(void *packet, int count)
{
    int i;
    unsigned char *p = (unsigned char *)packet;
    if (count > 999)
    {
        printf("Cannot print more than 999 chars\n");
        return;
    }
    printf("printing %d bytes... \n", count);
    printf("[NPK] [C] [HEX] [DEC] [BINARY]\n");
    printf("===============================\n");
    for (i = 0; i < count; i++)
    {
        printf("%3d | %c | %02X | %3d | %c%c%c%c%c%c%c%c\n", i, printable_char(p[i]), p[i], p[i],
               p[i] & 0x80 ? '1' : '0',
               p[i] & 0x40 ? '1' : '0',
               p[i] & 0x20 ? '1' : '0',
               p[i] & 0x10 ? '1' : '0',
               p[i] & 0x08 ? '1' : '0',
               p[i] & 0x04 ? '1' : '0',
               p[i] & 0x02 ? '1' : '0',
               p[i] & 0x01 ? '1' : '0');
    }
}

/*Conection*/
void start_network()
{
    int main_socket;
    int opt_value = 1;
    struct sockaddr_in server_address;
    int client_socket;
    struct sockaddr_in client_address;
    int client_address_size = sizeof(client_address);

    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket < 0)
    {
        printf("Error opening main server socket!\n");
        exit(1);
    }
    printf("Main socket created!\n");

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);
    setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt_value, sizeof(int));

    if (bind(main_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Error binding the main server socket!\n");
        exit(1);
    }
    printf("Main socket binded!\n");

    if (listen(main_socket, MAX_CLIENTS) < 0)
    {
        printf("error listening to socket!\n");
        exit(1);
    }
    printf("Main socket is listening\n");
    /*fflush(stdout);*/
    while (1)
    {
        int new_client_id = 0;
        int cpid = 0;

        client_socket = accept(main_socket, (struct socaddr *)&client_address, &client_address_size);
        if (client_socket < 0)
        {
            printf("Error accepting client connection! ERRNO=%d\n", errno);
            continue;
        }

        new_client_id = *client_count;
        *client_count += 1;
        cpid = fork();

        if (cpid == 0)
        {
            close(main_socket);
            cpid = fork();
            if (cpid == 0)
            {

                /*fflush(stdout);*/
                /*shared_clients[new_client_id].socket = client_socket;*/
                process_client(new_client_id, client_socket);
                printf("\ndo we get out of process client!\n");
                exit(0);
            }
            else
            {
                wait(NULL); /*might not need this line*/
                printf("Succesfully orphaned client %d\n", new_client_id);
                exit(0);
            }
        }
        else
        {
            close(client_socket);
        }
    }
}

/*these functions can take advantige of check endienes later just add if statement that chacks in which order they should work*/
int getPacketNumber(char *packet)
{
    int val = 0;

    int j = 0;
    int i;
    for (i = 3; i >= 0; --i)
    {
        val += (packet[i] & 0xFF) << (8 * j);
        ++j;
    }

    return val;
}

int getPacketID(char *packet)
{
    int val = 0;

    int j = 0;
    int i;
    for (i = 7; i >= 4; --i)
    {
        val += (packet[i] & 0xFF) << (8 * j);
        ++j;
    }

    return val;
}

long getPacketSize(char *packet)
{
    int val = 0;

    int j = 0;
    int i;
    /*15 8*/
    for (i = 8; i >= 5; --i)
    {
        val += (packet[i] & 0xFF) << (8 * j);
        ++j;
    }

    return val;
}

int getInt(char *packet)
{
    int val = 0;

    int j = 0;
    int i;
    for (i = 3; i >= 0; --i)
    {
        val += (packet[i] & 0xFF) << (8 * j);
        ++j;
    }

    return val;
}

long getLong(char *packet)
{
    long val = 0;

    int j = 0;
    int i;

    for (i = 7; i >= 0; --i)
    {
        val += (packet[i] & 0xFF) << (8 * j);
        ++j;
    }

    return val;
}


/*---------------------------------PACKET HENDLERS----------------------------------*/


void processPlayerInput(char* data, int size, int id){
    printf(" \n jeibogu \n");
    if(data[0] == '2'){
        printf("printe kkadu huinu");
        shared_clients[id].upKey = 1;
    }else if(data[0] == '1'){
        printf("printe kkadu huinu");
        shared_clients[id].downKey = 1;
    }else if(data[0] == '3'){
        printf("printe kkadu huinu");
        shared_clients[id].leftKey = 1;
    }else if(data[0] =='4'){
        printf("printe kkadu huinu");
        shared_clients[id].rightKey = 1;
    }else if(data[0]=='0'){
        printf("printe kkadu huinu");
        shared_clients[id].status = '0';
    }else if(data[0] == '5'){
        printf("printe kkadu huinu");
        /*shared_clients[id].status = 0;*/
        /*if you hit every part of your keybord that is your peoblem*/
    }

}

void processJoin(char* data, int size, int id){
    /*could do escaping but we kinda usless only makes the difference if things like player--name recieved we can just not allaw that*/
    printf("this is packet 1 join\n");
    int i;
    for(i = 0; (data + i) != '\n' && i< 20; i++){

        shared_clients[id].name[i] = *(data +i);
    }
    shared_clients[id].name[i + 1] = '\0';
    printf("check name %s \n", shared_clients[id].name);
}

void processMessage(char* data, int size, int id){
    printf("this is packet 3 messige\n");
    /*shared_clients[id].gameType = *data;*/


}

void processPlayerReady(char* data, int size, int id){
    printf("this is packet 6 player ready\n");
    shared_clients[id].status = 1; /*changes from 0 to 1 to indicate that player is ready packet 6*/
}

void processCheckStatus(char* data, int size, int id){
    printf("this is packet 9 player status\n");
/*empty for now - this packet is used to check if disconects happen*/
}

/*----------------------------------------------------------------------------------*/

int unwrapping(char *out, int id)
{
     /*unescaping packet */
    int ue;
    for(ue = 0; ue <= 1000; ue++){
        if(out[ue] == '?'){
            if(out[ue + 1] == '-'){
                out[ue] = '-';
                int i = ue + 1;
                for(i; i < 1000;i++){
                    out[i] = out[i+1];
                }

            }else if(out[ue + 1] == '*'){
                int i = ue + 1;
                for(i; i < 1000;i++){
                    out[i] = out[i+1];
                }
            }
        }
    }

    

    int PN = getPacketNumber(out);
    /*printf("PN from struct %i and PN from Package %i", shared_clients[id].PN, PN);*/
    if (PN <= shared_clients[id].PN){
        /*print_bytes(out, 35);*/
        printf("old packet recieved\n");
        return -1; /*will not process older packets*/
    }

    char ID = out[4];
    /*can check ID but overall Id will be more imoortant later*/

    int size = getPacketSize(out);
    print_bytes(out, size);
    /*int n = 0;*/
    /*daa mums nav beigas seperatora mes vinu jau nonemam taka poh ar checksum un PN checku pietiks var protams pachekot pec checksuma bet nu kada jega*/

    char CS = checksum(size+9, out);
    char CSP = out[size + 9];
    /*nu itka visam bet hz japateste*/

    /*printf("checksums are from packet - %i calculated - %i", CSP , CS);*/
    if(CS != CSP){
        printf("packet checksum is not correct\n");
        return -1;
    }
    /*printf("valid packet\n");*/


    /*printing*/
/*
    print_bytes(out, size + 10);
    printf("our id is %c", ID);
*/
    if(ID == '8'){
        processPlayerInput(&out[9], size, id);
    }else if(ID == '1'){
        processJoin(&out[9], size, id);
    }else if(ID == '3'){
        processMessage(&out[9], size, id);
    }else if(ID == '6'){
        processPlayerReady(&out[9], size, id);
    }else if(ID == '9'){
        processCheckStatus(&out[9], size, id);
    }else{
        printf("unknown packet recieved\n");
    }


    shared_clients[id].PN++;

    /* print_bytes(out, size); thiss will not print correctly because it starts withthe beggining of the packet not data segment*/
    /*printf("packet number : %d\npacket ID : %d\npacket size : %d\n ", PN, ID, size);*/

    /*
        shared_clients[id].PN += 1;
        fflush(stdout);
        char outputs[1024];

        int my_socket, payload_size;

        shared_clients[id].PN++;
        shared_clients[id].playerID = '9';
        payload_size = makeAccept(outputs, id);
        my_socket = shared_clients[id].socket;

        print_bytes(outputs , payload_size );
        send(my_socket, outputs, payload_size , 0);
        memset(outputs, 0, payload_size);

        usleep(1000 * 500);
    */


}

int toInt(char c) {
    /* vajag unsigned char*/
    return c - '0';
}

char toChar(int i) {
    return i + '0';
}

void reciever (int id, int socket){
    char in[1];
    int sepCounter = 0;
    int inpacket = 1;
    /*0 = ja', 1 = nē*/

 /*   printf("suka blet socket ir !!!!! -> %i <- !!!!!!\n", socket);*/

    int i = 0;
    printf("\n");
    while (1)
    {
        shared_clients[id].socket = socket;
        char out[1000];
        while (1)
        {
            shared_clients[id].socket = socket;
            read(socket, in, 1);
            if (inpacket == 0)
            {
                if (in[0] == '-' && out[i-1] != '?')
                {
                    /*checking if end of packet or just random dash*/
                    read(socket, in, 1);
                    if (in[0] == '-')
                    {
                        ++sepCounter;
                        if (sepCounter == 2)
                        {
                            sepCounter = 0;
                            inpacket = 1;

                            out[i] = '\0';
                            i = 0;
                            unwrapping(out, id);
                            memset(out, 0, 1000);
                            break;
                        }
                    }
                    else
                    {
                        out[i] = '-';
                        out[i + 1] = in[0];
                        i++;
                    }
                }
                else
                {
                    out[i] = in[0];
                }
                i++;
            }
            else
            {
                if (in[0] == '-')
                {
                    read(socket, in, 1);
                    if (in[0] == '-')
                    {
                        fflush(stdout);
                        ++sepCounter;
                        inpacket = 0;
                        break;
                    }
                }
            }
        }
    }
}

void writer (int id, int my_socket){

    sleep(80);
    int iterator = 0;
    while(1){
        
        
        /*strcpy(shared_clients[id].name, "dfjnvfsdnvsndf");*/
        /*print_bytes(outputs, payload_size);*
        /*sitos spagetus lugums apiet ar likumu - tadu jobanumu es vel nebiju ieprieks rakstijis*/
            /*escaping packet*/
        int g = 0;
        int es_size = 0;
        int client_packets_ready = 0;
        int ready_flag = 0;


        for(client_packets_ready; shared_clients[client_packets_ready].PNC >= iterator && *client_count <= client_packets_ready ; client_packets_ready++){    /*itterating client_packets_ready*/    }
        
        if (client_packets_ready == *client_count){
            ready_flag = 1;
            for(g; g <= *client_count ;g++){
                int ue;
                for(ue = 2; ue < shared_buffer[g].payload - 2; ue++){
                        if(shared_buffer[g].output[ue] == '?'){
                                int i = ue + 1;
                                char temp1;
                                char temp2;
                                for(i; i <= shared_buffer[g].payload;i++){
                                    if(i == ue+1){
                                        temp1 = shared_buffer[g].output[i+1];
                                        temp2 = shared_buffer[g].output[i+1];
                                        shared_buffer[g].output[i+1] = shared_buffer[g].output[i];
                                    }else{
                                    temp2 = shared_buffer[g].output[i+1];
                                    shared_buffer[g].output[i+1] = temp1;
                                    }
                                    temp1 = temp2;
                                }
                                shared_buffer[g].output[ue] = '?';
                                shared_buffer[g].output[ue+1] = '*';
                                ue++;
                                es_size++;
                            }else if(shared_buffer[g].output[ue] == '-'){
                                int i = ue + 1;
                                char temp1;
                                char temp2;
                                for(i; i <= shared_buffer[g].payload;i++){
                                    if(i == ue+1){
                                        temp1 = shared_buffer[g].output[i+1];
                                        temp2 = shared_buffer[g].output[i+1];
                                        shared_buffer[g].output[i+1] = shared_buffer[g].output[i];
                                    }else{
                                    temp2 = shared_buffer[g].output[i+1];
                                    shared_buffer[g].output[i+1] = temp1;
                                    }
                                    temp1 = temp2;
                                }
                                /*printf("WTF\n");*/
                                shared_buffer[g].output[ue] = '?';
                                shared_buffer[g].output[ue+1] = '-';
                                ue++;
                                es_size++;
                            }
                        }
            shared_buffer[g].payload = shared_buffer[g].payload + es_size;
            es_size = 0;
            }


        }
            
    

        
        
    /*sitos spagetus lugums apiet ar likumu - tadu jobanumu es vel nebiju ieprieks rakstijis -  bet vismaz tas strada*/
        /*print_bytes(outputs , payload_size + es_size);*/
        int suka = 0;
        for(suka; suka <= *client_count && ready_flag; suka++){
        send(my_socket, shared_buffer[suka].output, shared_buffer[suka].payload, 0);
        memset(shared_buffer[suka].output, 0, shared_buffer[suka].payload);
        usleep(1000 * 200);
        }

        suka = 0;
        g = 0;
        ready_flag = 0;
        client_packets_ready = 0;
        es_size = 0;

    }

}

void process_client(int id, int socket){
    int pid = fork();
    if (pid == 0){
        writer(id, socket);
    }
    else{
        reciever(id, socket);
    }
    /*Can write to char out[] untill the end -- is detected when detected send it to unwraper (also empty out[] for next packet) and wait untill -- detected again to start next packet*/
}

/*   ^^   process client is used in start network and it is started as separet process for reading socket so just write what you would write in listener directly into process_client        */

/*_____________________________________________________________________________________________________________*/

/*Game logics*/
void gameloop()
{
    sleep(10);
    printf("Started game loop! (it will run forever - use ctrl+C)\n");
    int i = 0;

    /*currently does not do shit - but it does not have to*/
    while (1)
    {
        int players_accepted = 0;

        for (i = 0; i < *client_count; i++)
        {
            printf("game loop exicutes client count is %i\n", *client_count);

            printf("tas ir vards ja %s\n", shared_clients[i].name);

            

            /*ifo te*/
            if(shared_clients[i].gameStatus ==  0){

                if(strlen(shared_clients[i].name) > 0 && strlen(shared_clients[i].name) < 21){
                    shared_clients[i].gameStatus++;
                    printf("inkremente game statusu - %i\n", shared_clients[i].gameStatus);
                }

                usleep(1000*100);
            }
            else if (shared_clients[i].gameStatus == 1){
                shared_clients[i].status = toChar(i);
                shared_clients[i].PNC++;
                printf("kas ir i %i\n", i);
                /*te talak ir sape*/
                shared_buffer[i].payload = 0;
                shared_buffer[i].payload = makeAccept(shared_buffer[i].output, i);
                print_bytes(shared_buffer[i].output, shared_buffer[i].payload);
                printf("nu bled un %i\n", shared_buffer[i].payload);
                fflush(stdout);
                usleep(1000*900);
                printf("make accept things in the house tonight");
            }
            else if (shared_clients[i].gameStatus == 2){
                printf("yolo 1");
            }
            else if (shared_clients[i].gameStatus == 3){
                printf("yolo 1");
            }
            else if (shared_clients[i].gameStatus == 4){
                printf("yolo 1");
            }

            /*  TODO:
            depending on game stage,
            1) Process lobby
            2) Decide to start the game
            3) In game - loop over inputs from all players, update gameworld
            4) Check if game ends
            */
          

        }
        sleep(1);
    }
}

/*Server main*/
int main(int argc, char **argv)
{
    printf("our little endian system :) :        %i \n", is_little_endian_system());
    int i = 0;
    if (strncmp("-p=", argv[1], 3) < 0)
    {
        printf("wrong parameter PORT %s \n", argv[2]);
        return -1;
    }
    char *realport;
    char *ranodmport = argv[1];
    for (i = 0; i < 2; i++)
        realport = strsep(&ranodmport, "=");
    port = atoi(realport);
    int pid = 0;
    printf("SERVER started!\n");
    get_shared_memory();

    pid = fork();
    if (pid == 0)
    {
        start_network();
    }
    else
    {
        gameloop();
    }

    return 0;
}