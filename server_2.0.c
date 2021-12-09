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
    /*join client*/
    char name[20];
    /*lobby server*/
    int status;             /*Player ready */
    char error[100];
    /*game type client*/
    int gameType; /*1v1 or 2v2*/
    /*player queue server ari int status*/
    /*game server ari int status*/
    /*player ready = empty*/
    /*game state server*/

    int scoreTeam1; int scoreTeam2;
    float playerX1; float playerY1;
    int playerHeight1; int playerWidth1;
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
struct Ball
{
    float ballX; float ballY;
    int ballRadius;
    int ballColor;
    int powerUpCount;
    int windowWidth; int windowHeight;
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
    char res=0;
    for(i = 0; i<length; i++){
        res ^= packet[i];
    }
    return res;
}


int makeAccept(char* pointer, char status, int id){
    char* buf = pointer;
    int PN = shared_clients[id].PN;

    addSep(buf);
    addInt(PN, (char*)buf+2);
    addChar(&buf[6], '2'); /* packetID*/
    addInt(20, &buf[7]); /*packet size*/
    add_string(status, &buf[11], strlen(status));
    char checkSum_Char = checksum(29, &buf[2]);
    addChar(&buf[31], checkSum_Char);
    addSep(&buf[32]);
    return 125;
}

int makePlayerQueue(char* pointer, int status, char* error, int id){
    char* buf = pointer;
    int PN = shared_clients[id].PN;
    addSep(buf);
    addInt(PN, (char*)buf+2);
    addInt(4, &buf[6]); /* packetID*/
    addLong(108, &buf[10]); /*packet size*/

    char checkSum_Char = checksum(104, &buf[19]);
    add_string(&checkSum_Char, &buf[18], 1);
    addInt(status, &buf[19]);
    add_string(error, &buf[23], strlen(error));
    addSep(&buf[123]);
    return 125;
}

int makeGameReady(char* pointer, int status, char* error, int id){
    char* buf = pointer;
    int PN = shared_clients[id].PN;
    addSep(buf);
    addInt(PN, (char*)buf+2);
    addInt(5, &buf[6]); /* packetID*/
    addLong(108, &buf[10]); /*packet size*/

    char checkSum_Char = checksum(104, &buf[19]);
    add_string(&checkSum_Char, &buf[18], 1);
    addInt(status, &buf[19]);
    add_string(error, &buf[23], strlen(error));
    addSep(&buf[123]);
    return 125;
}


int makeGameState(
    char* pointer,

    int windowWidth, int windowHeight, int scoreTeam1, int scoreTeam2, int gameType,
    float ballX, float ballY, int ballRadius, int ballColor, int powerUpCount,
    /*10*4 = 40 */
    int id, /*neskaitās, vajadzīgs, lai atrastu pareizo klientu */

    float playerX1, float playerY1, int playerHeight1, int playerWidth1, int playerColor1,
    float playerX2, float playerY2, int playerHeight2, int playerWidth2, int playerColor2,
    float playerX3, float playerY3, int playerHeight3, int playerWidth3, int playerColor3,
    float playerX4, float playerY4, int playerHeight4, int playerWidth4, int playerColor4,
    /* 20*4 = 80 */
    int ID1,  float powerUpX1, float powerUpY1, int powerUpWidth1, int powerUpHeight1,
    int ID2,  float powerUpX2, float  powerUpY2, int powerUpWidth2, int powerUpHeight2,
    int ID3, float powerUpX3, float powerUpY3, int powerUpWidth3, int powerUpHeight3
    /* 15*4 = 60 */
    ){
    char* buf = pointer;
    int PN = shared_clients[id].PN;
    addSep(buf);
    addInt(PN, (char*)buf+2);
    addInt(7, &buf[6]);
    addLong(180, &buf[10]);
/*start the data seg*/
    addInt(windowWidth, &buf[19]);
    addInt(windowHeight, &buf[23]);
    addInt(scoreTeam1, &buf[27]);
    addInt(scoreTeam2, &buf[31]);
    addInt(gameType, &buf[35]);
    addInt(ballX, &buf[39]);
    addInt(ballY, &buf[43]);
    addInt(ballRadius, &buf[47]);
    addInt(ballColor, &buf[51]);
    addInt(powerUpCount, &buf[55]);

    addInt(playerX1, &buf[59]);
    addInt(playerY1, &buf[63]);
    addInt(playerHeight1, &buf[67]);
    addInt(playerWidth1, &buf[71]);
    addInt(playerColor1, &buf[75]);
    addInt(playerX2, &buf[79]);
    addInt(playerY2, &buf[83]);
    addInt(playerHeight2, &buf[87]);
    addInt(playerWidth2, &buf[91]);
    addInt(playerColor2, &buf[95]);

    addInt(playerX3, &buf[99]);
    addInt(playerY3, &buf[103]);
    addInt(playerHeight3, &buf[107]);
    addInt(playerWidth3, &buf[111]);
    addInt(playerColor3, &buf[115]);
    addInt(playerX4, &buf[119]);
    addInt(playerY4, &buf[123]);
    addInt(playerHeight4, &buf[127]);
    addInt(playerWidth4, &buf[131]);
    addInt(playerColor4, &buf[135]);

    addInt(ID1, &buf[139]);
    addInt(powerUpX1, &buf[143]);
    addInt(powerUpY1, &buf[147]);
    addInt(powerUpWidth1, &buf[151]);
    addInt(powerUpHeight1, &buf[155]);
    addInt(ID2, &buf[159]);
    addInt(powerUpX2, &buf[163]);
    addInt(powerUpY2, &buf[167]);
    addInt(powerUpWidth2, &buf[171]);
    addInt(powerUpHeight2, &buf[175]);
    addInt(ID3, &buf[179]);
    addInt(powerUpX3, &buf[183]);
    addInt(powerUpY3, &buf[187]);
    addInt(powerUpWidth3, &buf[191]);
    addInt(powerUpHeight3, &buf[195]);

    /*end the data seg*/
    char checkSum_Char = checksum(180, &buf[19]);
    add_string(&checkSum_Char, &buf[18], 1);
    addSep(&buf[200]);
    return 202;
}

int makeGameEnd (char* pointer, int status, char* error, int playerTeamScore, int enemyTeamScore,
    int gameDuration, char* mvp, char* name1, int numberOfGoals1, char* name2, int numberOfGoals2,
    char *name3, int numberOfGoals3, char* name4, int numberOfGoals4
){

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
        shared_clients = (struct Client *)(sizeof(shared_balls) + shared_memory);

        /*initializing objects*/
        *client_count = 0; /*NOT SURE ABOUT THE ORDER -1 or 0*/
        struct Ball gameball = *shared_balls;
        int c_iterator = 0;
        for (c_iterator; c_iterator < MAX_CLIENTS; c_iterator++)
        {
            struct Client cl = shared_clients[c_iterator];
            cl.PN = -1;
            cl.status = 0; /*not ready*/
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
    printf("Main socket is listening");
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
                process_client(new_client_id, client_socket);
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


processPlayerInput(char* data, int size, int id){
    if(!strcmp(data, '2')){
        shared_clients[id].upKey = 1;

    }else if(!strcmp(data, '1')){
        printf("printe kkadu huinu");
        shared_clients[id].downKey = 1;
    }else if(!strcmp(data, '3')){
        printf("printe kkadu huinu");
        shared_clients[id].leftKey = 1;
    }else if(!strcmp(data, '4')){
        printf("printe kkadu huinu");
        shared_clients[id].rightKey = 1;
    }else if(!strcmp(data, '0')){
        printf("printe kkadu huinu");
        shared_clients[id].status = 0;
    }else if(!strcmp(data, '5')){
        printf("printe kkadu huinu");
        /*shared_clients[id].status = 0;*/
        /*if you hit every part of your keybord that is your peoblem*/
    }

}

processJoin(char* data, int size, int id){
    /*could do escaping but we kinda usless only makes the difference if things like player--name recieved we can just not allaw that*/
    int i;
    for(i = 0; (data + i) != '\n' && i< 20; i++){
        shared_clients[id].name[i] = data +i;
    }
    printf("check name %s", shared_clients[id].name);

}

processGameType(char* data, int size, int id){
    shared_clients[id].gameType = data;
}

processPlayerRedy(char* data, int size, int id){
    shared_clients[id].status = 1; /*changes from 0 to 1 to indicate that player is ready packet 6*/
}

processCheckStatus(char* data, int size, int id){
/*empty for now - this packet is used to check if disconects happen*/
}

/*----------------------------------------------------------------------------------*/

void unwrapping(char *out, int id)
{

    int PN = getPacketNumber(out);
    if (PN < shared_clients[id].PN){
        printf("old packet recieved\n");
        return; /*will not process older packets*/
    }
/*
char checksum(int length, char* packet){
    int i;
    char res=0;
    for(i = 0; i<length; i++){
        res ^= packet[i];
    }
    return res;
}
*/
    int ID = out[4];
    /*can check ID but overall Id will be more imoortant later*/
    int size = getPacketSize(out);
    /*int n = 0;*/
    /*daa mums nav beigas seperatora mes vinu jau nonemam taka poh ar checksum un PN checku pietiks var protams pachekot pec checksuma bet nu kada jega*/
    printf("jobans ar biezpienu 2.0 %i \n", size);
    fflush(stdout);
    char CS = checksum(size+9, out[0]);

    char CSP = out[size + 9];

    /*nu itka visam bet hz japateste*/

    printf("checksum calculated %c\necieved calculated %c\n", CS, CSP);
    if(CS != CSP){
        printf("packet checksum is not correct\n");
        return;
    }
    printf("valid packet\n");

    /*unescaping packet pagaidam ne jo nav nehuja skaidrs*/
    /*
    int ue;
    for(ue = 0; ue <= size + 10; ue++){
        if(out[ue] == '?'){
            if(out[ue + 1] == kas ir? es tevi nedzirdu, viss lab?)
        }
    }
    */

    /*printing*/
    print_bytes(out, size + 17);

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

    /* print_bytes(out, size); thiss will not print correctly because it starts withthe beggining of the packet not data segment*/
    /*printf("packet number : %d\npacket ID : %d\npacket size : %d\n ", PN, ID, size);*/
    shared_clients[id].PN += 1;
    fflush(stdout);

}

void process_client(int id, int socket)
{
    char in[1];
    int sepCounter = 0;
    int inpacket = 1;
    /*0 = ja', 1 = nē*/

    int i = 0;
    printf("\n");
    while (1)
    {
        char out[1000];
        while (1)
        {
            read(socket, in, 1);
            if (inpacket == 0)
            {
                if (in[0] == '-')
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
            /*
            shared_clients[MAX_CLIENTS +i];
            shared_clients[i] = 0;
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
    printf("tiek līdz šejienei");
    fflush(stdout);
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