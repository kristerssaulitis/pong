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
int *client_count = NULL; /*client count starts from 0 so to get client count <- client_count + 1*/
struct Ball *shared_balls = NULL;
struct Client *shared_clients = NULL;

char checksum(int length, char *packet);

/*Shared memory structures*/ /*We might need to reset some values to default because they get initialized with some trash*/
struct Client
{
    int PN;
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

    /*check status ik pec 5 sec*/
    /*game end server*/
    int gameDuration;
};
struct Ball
{
    float ballX; float ballY;
    int ballRadius; int ballColor;
    char ballCount;

    float team_goal1X; float team_goal1Y;
    float team_goal2X; float team_goal2Y;
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
    int PN = shared_clients[id].PN;
    char playerID = shared_clients[id].playerID;

    addSep(buf); /*2*/
    addInt(PN, &buf[2]); /*6*/
    addChar(&buf[6], '2'); /* packetID 7*/
    addInt(1, &buf[7]); /*packet size 11*/
    addChar(&buf[11], playerID); /*12*/
    char checkSum_Char = checksum( 10 , &buf[2]);
    addChar(&buf[12], checkSum_Char); /*13*/
    addSep(&buf[13]);
    return 15;
}

int makeMessage(char* pointer, char* message, int id){
    char* buf = pointer;
    int PN = shared_clients[id].PN;
    char targetID = shared_clients[id].targetID;
    char sourceID = 'a';
    /*[0=chat, 1=info, 2=error]*/
    addSep(buf); /*2*/
    addInt(PN, (char*)buf+2); /*6*/
    addChar(&buf[6], '3'); /* packetID 7*/
    addInt(258, &buf[7]); /*packet size 11*/
    addChar(&buf[11], targetID); /*12*/
    addChar(&buf[12], sourceID); /*13*/
    add_string(message, &buf[13], 256); /*256+13 = 269*/
    char checkSum_Char = checksum(267, &buf[2]); /**/
    addChar(&buf[269], checkSum_Char); /*270*/
    addSep(&buf[270]); /*270*/
    return 272;
}

int makeLobby(char* pointer,  int id){
    char* buf = pointer;
    int PN = shared_clients[id].PN;
    char name[20];
    strcpy(name,shared_clients[id].name);
    char count = shared_balls->playerCount;
    char playerID = shared_clients[id].playerID;

    addSep(buf); /*2*/
    addInt(PN, &buf[2]); /*6*/
    addChar(&buf[6], '4'); /* packetID 7*/
    addInt(22, &buf[7]); /*packet size 11*/
    addChar(&buf[11], count); /*12*/
    addChar(&buf[12], playerID);
    add_string(name, &buf[13], 20);
    char checkSum_Char = checksum(31, &buf[2]);
    addChar(&buf[32], checkSum_Char); /*13*/
    addSep(&buf[33]);
    return 35;
}

int makeGameReady( char* pointer, int id ){
    char* buf = pointer;

    int PN = shared_clients[id].PN;
    int windowWidth = shared_balls->windowWidth;
    int windowHeight = shared_balls->windowHeight;
    char teamCount = shared_balls->teamCount;
    char teamID = shared_clients[id].teamID;
    float team_goal1X = shared_balls->team_goal1X;
    float team_goal1Y = shared_balls->team_goal1Y;
    float team_goal2X = shared_balls->team_goal2X;
    float team_goal2Y = shared_balls->team_goal2Y;
    char ready = shared_clients[id].ready;
    char name[20];
    strcpy(name,shared_clients[id].name);
    float playerX1 = shared_clients[id].playerX1;
    float playerY1 = shared_clients[id].playerY1;
    int playerHeight1 = shared_clients[id].playerHeight1;
    int playerWidth1 = shared_clients[id].playerWidth1;
    char playerCount = shared_balls->playerCount;
    char playerID = shared_clients[id].playerID;

    addSep(buf);
    addInt(PN, &buf[2]); /*6*/
    addChar(&buf[6], '5'); /* packetID 7*/
    addInt(66, &buf[7]); /*packet size 11*/
    addInt(windowWidth, &buf[11]);
    addInt(windowHeight, &buf[15]);
    addChar(&buf[19], teamCount);
    addChar(&buf[20], teamID);
    addInt(team_goal1X, &buf[21]);
    addInt(team_goal1Y, &buf[25]);
    addInt(team_goal2X, &buf[29]);
    addInt(team_goal2Y, &buf[33]);
    addChar(&buf[37], playerCount);
    addChar(&buf[38], playerID);
    addChar(&buf[39], ready);
    add_string(name, &buf[40], 20);
    addInt(playerX1, &buf[60]);
    addInt(playerY1, &buf[64]);
    addInt(playerWidth1, &buf[68]);
    addInt(playerHeight1, &buf[72]);
    char checkSum_Char = checksum(75, &buf[2]);
    addChar(&buf[76], checkSum_Char); /*13*/
    addSep(&buf[77]);
    return 79;
}

int makeGameState( char* pointer, int id ){
    char* buf = pointer;

    int PN = shared_clients[id].PN;
    int windowWidth = shared_balls->windowWidth;
    int windowHeight = shared_balls->windowHeight;
    char teamCount = shared_balls->teamCount;
    char teamID = shared_clients[id].teamID;
    int scoreTeam = shared_clients[id].scoreTeam;
    float team_goal1X = shared_balls->team_goal1X;
    float team_goal1Y = shared_balls->team_goal1Y;
    float team_goal2X = shared_balls->team_goal2X;
    float team_goal2Y = shared_balls->team_goal2Y;
    float playerX1 = shared_clients[id].playerX1;
    float playerY1 = shared_clients[id].playerY1;
    int playerHeight1 = shared_clients[id].playerHeight1;
    int playerWidth1 = shared_clients[id].playerWidth1;
    char playerCount = shared_balls->playerCount;
    char ballCount = shared_balls->ballCount;
    float ballX = shared_balls->ballX;
    float ballY = shared_balls->ballY;
    int ballRadius = shared_balls->ballRadius;
    int ballColor = shared_balls->ballColor;
    char powerUpCount = shared_balls->powerUpCount;
    char powerUpType = shared_balls->powerUpType;
    float powerUpX1 = shared_balls->powerUpX1;
    float powerUpY1 = shared_balls->powerUpY1;
    int powerUpWidth1 = shared_balls->powerUpWidth1;
    int powerUpHeight1 = shared_balls->powerUpHeight1;
    char playerID = shared_clients[id].playerID;

    addSep(buf);
    addInt(PN, &buf[2]); /*6*/
    addChar(&buf[6], '7'); /* packetID 7*/
    addInt(81, &buf[7]); /*packet size 11*/
/*start the data seg*/
    addInt(windowWidth, &buf[11]);
    addInt(windowHeight, &buf[15]);
    addChar(&buf[19], teamCount);
    addChar(&buf[20], teamID);
    addInt(scoreTeam, &buf[21]);
    addInt(team_goal1X, &buf[25]);
    addInt(team_goal1Y, &buf[29]);
    addInt(team_goal2X, &buf[33]);
    addInt(team_goal2Y, &buf[37]);
    addChar(&buf[41], playerCount);
    addChar(&buf[42], playerID);
    addChar(&buf[43], teamID);
    addInt(playerX1, &buf[44]);
    addInt(playerY1, &buf[48]);
    addInt(playerHeight1, &buf[52]);
    addInt(playerWidth1, &buf[56]);
    addChar(&buf[60], ballCount);
    addInt(ballX, &buf[61]);
    addInt(ballY, &buf[65]);
    addInt(ballRadius, &buf[69]);
    addChar(&buf[73], ballColor);
    addChar(&buf[74], powerUpCount);
    addChar(&buf[74], powerUpType);
    addInt(powerUpX1, &buf[75]);
    addInt(powerUpY1, &buf[79]);
    addInt(powerUpWidth1, &buf[83]);
    addInt(powerUpHeight1, &buf[87]);
    /*end the data seg*/
    char checkSum_Char = checksum(90, &buf[2]);
    addChar(&buf[91], checkSum_Char); /*13*/
    addSep(&buf[92]);
    return 94;
}

int makeGameEnd (char* pointer, int id, char status ){
    char* buf = pointer;

    int PN = shared_clients[id].PN;
    int scoreTeam = shared_clients[id].scoreTeam;
    int gameDuration = shared_balls->gameDuration;
    char teamCount = shared_balls->teamCount;
    char teamID = shared_clients[id].teamID;
    char playerCount = shared_balls->playerCount;
    char playerID = shared_clients[id].playerID;
    char name[20];
    strcpy(name,shared_clients[id].name);
    int score = shared_clients[id].score;

    addSep(buf);
    addInt(PN, &buf[2]); /*6*/
    addChar(&buf[6], '10'); /* packetID 7*/
    addInt(42, &buf[7]); /*packet size 11*/
/*start the data seg*/
    addChar(&buf[11], status);
    addInt(scoreTeam, &buf[12]);
    addInt(gameDuration, &buf[16]);
    addChar(&buf[18], teamCount);
    addChar(&buf[19], teamID);
    addInt(score, &buf[20]);
    addChar(&buf[24], playerCount);
    addChar(&buf[25], playerID);
    addChar(&buf[26], teamID);
    addInt(score, &buf[27]);
    add_string(name, &buf[31], 20);
    /*end the data seg*/
    char checkSum_Char = checksum(51, &buf[2]);
    addChar(&buf[51], checkSum_Char); /*13*/
    addSep(&buf[52]);
    return 54;
}
/*pretty functions*/
/*_____________________________________________________________________________________________________________*/
/*shared memory*/
void get_shared_memory()
{
    int sizeofthings = sizeof(struct Ball) + MAX_CLIENTS * sizeof(struct Client) + sizeof(int);     /*size of int might not be needed initially wanted to use for PN but its better to store it in client struct now acts like a protective buffer I guess can be deleted*/
    if (shared_memory = mmap(NULL, sizeofthings, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0))
    {
        client_count = shared_memory;
        shared_balls = (struct Ball *)shared_memory + sizeof(int);
        shared_clients = (struct Client *)(sizeof(shared_balls) + shared_balls);

        /*initializing objects*/
        *client_count = 0; /*NOT SURE ABOUT THE ORDER -1 or 0*/
        struct Ball gameball = *shared_balls;
        int c_iterator = 0;
        for (c_iterator; c_iterator < MAX_CLIENTS; c_iterator++)
        {
            struct Client cl = shared_clients[c_iterator];
            cl.PN = -1;
            cl.status = '0'; /*not ready*/
        }

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
    char *p = (char *)packet;
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
    int i;
    for(i = 0; (data + i) != '\n' && i< 20; i++){

        shared_clients[id].name[i] = *(data +i);
    }
    shared_clients[id].name[i + 1] = '\0';
    printf("check name %s \n", shared_clients[id].name);

}

void processGameType(char* data, int size, int id){
    shared_clients[id].gameType = data;
}

void processPlayerRedy(char* data, int size, int id){
    shared_clients[id].status = 1; /*changes from 0 to 1 to indicate that player is ready packet 6*/
}

void processCheckStatus(char* data, int size, int id){
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

    /*print_bytes(out, 40);*/

    int PN = getPacketNumber(out);
    /*printf("PN from struct %i and PN from Package %i", shared_clients[id].PN, PN);*/
    if (PN <= shared_clients[id].PN){
        print_bytes(out, 35);
        printf("old packet recieved\n");
        return -1; /*will not process older packets*/
    }

    char ID = out[4];
    /*can check ID but overall Id will be more imoortant later*/

    int size = getPacketSize(out);
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
        processGameType(&out[9], size, id);
    }else if(ID == '6'){
        processPlayerRedy(&out[9], size, id);
    }else if(ID == '9'){
        processCheckStatus(&out[9], size, id);
    }else{
        printf("unknown packet recieved\n");
    }

    /* Should not be here but in game loop*/

    /* print_bytes(out, size); thiss will not print correctly because it starts withthe beggining of the packet not data segment*/
    /*printf("packet number : %d\npacket ID : %d\npacket size : %d\n ", PN, ID, size);*/
    shared_clients[id].PN += 1;
    fflush(stdout);

}

reciever (int id, int socket){
    char in[1];
    int sepCounter = 0;
    int inpacket = 1;
    /*0 = ja', 1 = nē*/

    printf("suka blet socket ir !!!!! -> %i <- !!!!!!\n", socket);

    int i = 0;
    printf("\n");
    while (1)
    {
        shared_clients[id].socket = socket;
        printf("suka blet socket ir !!!!! -> %i <- !!!!!!\n", socket);
        char out[1000];
        while (1)
        {
            shared_clients[id].socket = socket;
            printf("suka blet socket ir !!!!! -> %i <- !!!!!!\n", socket);
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
                            memset(out, 0, 8);
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

writer (int id, int socket){
    int payload_size = 0;
    char outputs[1024];
    int my_socket = 0;
    shared_clients[id].playerID = '9';
    payload_size = makeAccept(outputs, id);
    my_socket = socket;
    printf("the client whom to send %i and its socket %i | total client count %i\n", id ,my_socket, *client_count);

    print_bytes(outputs , payload_size);
    write(my_socket, outputs, payload_size);
    memset(outputs, 0, payload_size);

    printf("payload size is %i", payload_size);
    write(socket,outputs,payload_size);
    fflush(stdout);
}

void process_client(int id, int socket){
    int pid = fork();
    if (pid == 0){
        reciever(id, socket);
    }
    else{
        writer(id, socket);
    }
    /*Can write to char out[] untill the end -- is detected when detected send it to unwraper (also empty out[] for next packet) and wait untill -- detected again to start next packet*/
}

/*   ^^   process client is used in start network and it is started as separet process for reading socket so just write what you would write in listener directly into process_client        */

/*_____________________________________________________________________________________________________________*/

/*Game logics*/
void gameloop()
{
    printf("Started game loop! (it will run forever - use ctrl+C)\n");
    int i = 0;

    /*currently does not do shit - but it does not have to*/
    while (1)
    {


        for (i = 0; i < *client_count; i++)
        {
            printf("game loop exicutes client count is %i", *client_count);
            /*  TODO:
            depending on game stage,
            1) Process lobby
            2) Decide to start the game
            3) In game - loop over inputs from all players, update gameworld
            4) Check if game ends
            */
           /*
            int payload_size = 0;
            char outputs[1024];
            int my_socket = 0;
            shared_clients[i].playerID = '9';
            payload_size = makeAccept(outputs, i);
            my_socket = shared_clients[i].socket;
            printf("the client whom to send %i and its socket %i | total client count %i\n", i ,my_socket, *client_count);

            print_bytes(outputs , payload_size);
            write(my_socket, outputs, payload_size);
            memset(outputs, 0, payload_size);

            printf("payload size is %i", payload_size);
            write(socket,outputs,payload_size);
            fflush(stdout);
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