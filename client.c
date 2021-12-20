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
#include <ncurses.h>
#include <sys/mman.h>

#define MAXSIZE 1024

char* host;
int port;
struct sockaddr_in remote_address;


         /*global variables*/
/*----------------------------------*/
void* shared_memory = NULL;
char* outputs = NULL;
int* payload_size = NULL;
struct Client *myClient = NULL;
struct Object* scr = NULL;
struct Object* ball = NULL;
struct Object* player1 = NULL;
struct Object* player2 = NULL;
struct Inputs* inp = NULL;
/*for client to understand at which state it is*/


struct Client {
    int PN;
    int PNS;
    char name[20];
    char targname[20];
    char playerID;
    char targetID;
    char gameType;
    char teamID;
    char ready;
    char message[256];

    int scoreTeam;
    int score;
    int playerX1; int playerY1;
    int playerX2; int playerY2;
    int playerHeight1; int playerWidth1;
    int playerColor1;

    /*game state server*/
    char upKey;
    char downKey;
    char leftKey;
    char rightKey;
    char exit;
    char action;

    float ballX; float ballY;
    int ballRadius; int ballColor;
    char ballCount;

    float team_goal1X; float team_goal1Y;
    float team_goal2X; float team_goal2Y;
    char teamCount; char playerCount;
    int powerUpCount; char powerUpType;
    int windowWidth; int windowHeight;
    int gameDuration;
    float powerUpX1; float powerUpY1;
    int powerUpWidth1; int powerUpHeight1;
};

struct Object{
    short int x, y, c;
    bool movhor, movver, end;
} ;

struct Inputs{
    char c;
    bool end, pause;
} ;
/*----------------------------------*/

void *connection_handler(void* socket_desc);
void addSep( char * buf);
void addInt(int num,  char * buf);
void addLong(long num,  char * buf);
void add_string(char* str,  char* buf, int count);
char checksum(int length, char* packet);
int makeJoin( char* pointer);
int makePlayerInput(char* pointer, char input);
int gameloop();
int keyreading();
void drawer();

int toInt(char c) {
    /* vajag unsigned char*/
    return c - '0';
}

char toChar(int i) {
    return i + '0';
}

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

void processAccept(char* out, int size){
    char c = out[9];
    printf("this is accept\n");
    /*print_bytes(out, 11);*/

    if (c == '-1'){
        printf("viss nav bumbas %c", c);
        exit(1);
    } else {
        printf("viss ir bumbas %c", c);
        myClient->playerID = c;
    }
}

void processMessage(char* out, int size){
    printf("this is message\n");
}

void processLobby(char* out, int size){
    printf("CLIENT this is lobby\n");
    /*print_bytes(out, 20);*/
    int playCount = toInt(out[9]);
    char playID[playCount];
    char allNameiz[playCount][20];
    int current = 10;
    int i = 0, z =0;
    for (i; i< playCount; i++){
        playID[i] = out[current++];
        for(z=0; (out + z) != '\n' && z< 20; z++){
            allNameiz[i][z] = out[current++];
        }
    }
    if (myClient->playerID == '0'){
        myClient->playerID = playID[0];
        strcpy(myClient->name, allNameiz[0]);
        printf("this is playerID %c, this is its name %s", myClient->playerID, myClient->name);
        if (playCount>1) {
            myClient->targetID = playID[1];
            strcpy(myClient->targname, allNameiz[1]);
        }
    } else {
        myClient->targetID = playID[0];
        strcpy(myClient->targname, allNameiz[0]);
        myClient->playerID = playID[1];
        strcpy(myClient->name, allNameiz[1]);
        printf("this is playerID %c, this is its name %s", myClient->playerID, myClient->name);

    }
    

}

void processGameReady(char* out, int size){
    printf("this is game ready\n");
    
    int playCount = toInt(out[9]);
    char playID[playCount];
    char teamID[playCount];
    char ready[playCount];
    char allNameiz[playCount][20];
    int x[playCount];
    int y[playCount];
    int current = 10;
    int i = 0;
    for (i; i< playCount; i++){
        playID[i] = out[current++];
        ready[i] = out[current++];
        teamID[i] = out[current++];
        current +=20;
        x[i] = out[current +=4];
        y[i] = out[current +=4];
        current +=8;
    }

    if (myClient->playerID == '0'){
        myClient->playerID = playID[0];
        myClient->ready = ready[0];
        myClient->playerX1 = x[0];
        myClient->playerY1 = x[0];
        printf("this is playerID %c, this is its name %s", myClient->playerID, myClient->name);
        if (playCount>1) {
            myClient->targetID = playID[1];
            myClient->playerX2 = x[1];
            myClient->playerY2 = x[1];
        }
    } else {
        myClient->playerID = playID[1];
        myClient->ready = ready[1];
        myClient->playerX1 = x[1];
        myClient->playerY1 = x[1];
        myClient->targetID = playID[0];
        myClient->playerX2 = x[0];
        myClient->playerY2 = x[0];
    }
    
}

void processGameState(char* out, int size){
    printf("this is game state\n");
}

void processGameEnd(char* out, int size){
    printf("this is game end\n");
}

int unwrapping(char *out){
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
    if (PN <= myClient->PN){
        /*print_bytes(out, 11);*/
        /*printf("old packet recieved\n");*/
        return -1;
    }

    /*print_bytes(out, 11);*/

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
    printf("valid packet\n");
    /*koment share subscribe*/
    if(ID == '2'){
        printf(" packet recieved 2\n");
        processAccept(out, size);
    }else if(ID == '3'){
        printf(" packet recieved 3\n");
        processMessage(out, size);
    }else if(ID == '7'){
        printf(" packet recieved 7\n");
        processGameState(out, size);
    }else if(ID == '5'){
        printf(" packet recieved 5\n");
        processGameReady(out, size);
    }else if(ID == '4'){
        printf(" packet recieved 4\n");
        processLobby(out, size);
    }else if (ID == '10'){
        printf(" packet recieved 10\n");
        processGameEnd(out, size);
    }
    else{
        printf("unknown packet recieved\n");
    }

    /* Should not be here but in game loop*/

    /* print_bytes(out, size); thiss will not print correctly because it starts withthe beggining of the packet not data segment*/
    /*printf("packet number : %d\npacket ID : %d\npacket size : %d\n ", PN, ID, size);*/
    myClient->PN += 1;
    fflush(stdout);

}

void reader(int my_sock){
    char in[1];
    int sepCounter = 0;
    int inpacket = 1;
    /*0 = ja', 1 = nē*/
    int i = 0;
    printf("\n");
    while (1)
    {
        char out[1000];
        while (1){
            read(my_sock, in, 1);
            if (inpacket == 0)
            {

                if (in[0] == '-' && out[i-1] != '?')
                {

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
                            /*print_bytes(out, 15);*/
                            unwrapping(out);
                            /*memset(out, 0, 1000);*/
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
                    read(my_sock, in, 1);
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
        usleep(1000*100);
    }
}

void writer(int my_sock){
    int itteration = myClient->PNS;
    while(1){
        /*for sending from client to server*/
        if(myClient->PNS <= itteration){
            /*printf("gaidaaaaam! un iteration ir %i bet PN ir %i\n", itteration, myClient->PNS);*/
            usleep(1000*600);
        }else{
        itteration = myClient->PNS;
        printf("sakam! un payload size ir %i\n", *payload_size);

        /*sitos spagetus lugums apiet ar likumu - tadu jobanumu es vel nebiju ieprieks rakstijis*/
            /*escaping packet*/
        int ue;
        int es_size = 0;
        for(ue = 2; ue < *payload_size - 2; ue++){
                if(outputs[ue] == '?'){
                        int i = ue + 1;
                        char temp1;
                        char temp2;
                        for(i; i <= *payload_size;i++){
                            if(i == ue+1){
                                temp1 = outputs[i+1];
                                temp2 = outputs[i+1];
                                outputs[i+1] = outputs[i];
                            }else{
                            temp2 = outputs[i+1];
                            outputs[i+1] = temp1;
                            }
                            temp1 = temp2;
                        }
                        outputs[ue] = '?';
                        outputs[ue+1] = '*';
                        ue++;
                        es_size++;
                    }else if(outputs[ue] == '-'){
                        int i = ue + 1;
                        char temp1;
                        char temp2;
                        for(i; i <= *payload_size;i++){
                            if(i == ue+1){
                                temp1 = outputs[i+1];
                                temp2 = outputs[i+1];
                                outputs[i+1] = outputs[i];
                            }else{
                            temp2 = outputs[i+1];
                            outputs[i+1] = temp1;
                            }
                            temp1 = temp2;
                        }
                        /*printf("WTF\n");*/
                        outputs[ue] = '?';
                        outputs[ue+1] = '-';
                        ue++;
                        es_size++;
                    }
                }
    /*sitos spagetus lugums apiet ar likumu - tadu jobanumu es vel nebiju ieprieks rakstijis -  bet vismaz tas strada*/
        /*print_bytes(outputs, *payload_size + es_size);*/
        send(my_sock,outputs, *payload_size + es_size,0);
        memset(outputs, 0, *payload_size +es_size);
        /*payload_size = 0;*/
        es_size = 0;


        usleep(1000* 200);
    }}
    return;
    }

int keyreading(){
    int ch;
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    while(1){
        ch = getch();
        switch(ch){
            case KEY_UP:
                printw("\n UP Arrow");
                break;
            case KEY_DOWN:
                printw("\n DOWN Arrow");
                break;
            case KEY_LEFT:
                printw("\n LEFT Arrow");
                break;
            case KEY_RIGHT:
                printw("\n RIGHT Arrow");
                break;
            case KEY_EXIT:
                printw("\n EXIT Arrow");
                break;
            default:
                printw("\n The pressed key is %c", ch);
        }
    }

    endwin();
    return 0;
}



int gameloop(){
    int state = 0;
    printf("pilnigs suds");

    while(1){
        /*keyreading();*/
        if(state ==  0){
            char name[20];
            printf("kads ir jusu nickname?\n");
            scanf("%s", name);
            strcpy(myClient->name, name);
            printf("\nthis is your name %s\n", myClient->name);
            myClient->PNS += 1;
            printf("the PNS is %i\n", myClient->PNS);
            /*memset(outputs, 0, MAXSIZE);*/    /*es nezinu kapec bet saja pakete ir daudz lieka garbage ta jau itka funkcionalitatei paslaik nemaisa un viss lasas pareizi bet nezinu ka izlabot memsets nepalidz*/
            *payload_size = makeJoin(outputs);
            printf("state 0\n");
        }
        else if (state == 1){

            printf("yolo 1");
        }
        else if (state == 2){
            printf("game ready atsutits no serverA 1");
            initscr();
    start_color();
    init_pair(1,COLOR_BLUE,COLOR_BLACK);
    keypad(stdscr,true);
    noecho();
    curs_set(0);
    getmaxyx(stdscr,scr->y,scr->x);
    player1->x= scr->x-2;
    player1->y= scr->y/2;
    player1->c= 0;
    player1->movver = false;
    player1->movhor = false;
    player1->end = false;

    player2->x= 1;
    player2->y= scr->y/2;
    player2->c= 0;
    player2->movver = false;
    player2->movhor = false;
    player2->end = false;

    ball->x= scr->x/2;
    ball->y= scr->y/2;
    ball->c= 0;
    ball->movver = false;
    ball->movhor = false;
    ball->end = false;
    drawer();
        }
        else if (state == 3){
            printf("yolo 1");
        }
        else if (state == 4){
            printf("yolo 1");
        }
        usleep(1000*200);
    }
}
/*Packet functions*/
/*Note - although I coppied here 10 functions the actual count for client is less some functions are used by client but some by server so it will change*/


void drawer(){
    mvprintw(4,0,
        "\t \t\t\tPlayer 1 your controls are up and down arrow         \n"
        "\t \t\t\tPush ANY key to start, 'p' for pause and ESC to quit" );

    getch();
    for (nodelay(stdscr,1); !inp->end; usleep(4000)) {

        /*šeit sākas input reading*/
        switch (getch()) {
        case KEY_DOWN: inp->c = 'd'; break;
        case KEY_UP:   inp->c = 'u'; break;
        case 'p':      getchar();inp->c = 'p'; break;
        case 0x1B:    endwin(); inp->c = 'e'; break;
        }
        myClient->PNS += 1;
        *payload_size = makePlayerInput(outputs, inp->c);
        usleep(1000* 200);
        erase();
        mvprintw(2,scr->x/2-2,"%i | %i",player1->c,player2->c); /*scoreboard*/
        mvvline(0,scr->x/2,ACS_VLINE,scr->y); /*videja linija*/
        attron(COLOR_PAIR(1));
        mvprintw(ball->y,ball->x,"o");
        int i;
        for(i=-1;i<2;i++){
            mvprintw(player1->y+i,player1->x,"|");
            mvprintw(player2->y+i,player2->x,"|");
        }
        attroff(COLOR_PAIR(1));
    }
}


int makeJoin(char* pointer ){
    char* buf = pointer;
    int PN = myClient->PNS;
    char name[20];
    int ret = 0;
    strcpy(name,myClient->name);

    addSep(buf); ret +=2;
    addInt(PN, (char*)buf+ret); ret +=4;
    addChar(&buf[ret], '1'); /* packetID*/ ret +=1;
    addInt(20, &buf[ret]); /*packet size*/ ret +=4;
    add_string(name, &buf[ret], 20); ret +=20;
    char checkSum_Char = checksum(ret-2, &buf[2]);
    addChar(&buf[ret], checkSum_Char); ret +=1;
    addSep(&buf[ret]); ret +=2;
    return ret;
}

int makeMessage(char* pointer ){
    char* buf = pointer;
    int PN = myClient->PNS;
    int ret = 0;
    char message[256];
    strcpy(message,myClient->name);
    char playerID = myClient->playerID;

    addSep(buf); /*2*/ ret +=2;
    addInt(PN, &buf[ret]); /*6*/ ret +=4;
    addChar(&buf[ret], '3'); /* packetID 7*/ ret +=1;
    addInt(258, &buf[ret]); /*packet size 11*/ ret +=4;
    addChar(&buf[ret], playerID); /*12*/ ret +=1;
    addChar(&buf[ret], playerID); /*13*/ ret +=1;
    add_string(message, &buf[ret], 256); /*256+13 = 269*/ ret +=256;
    char checkSum_Char = checksum(ret-2, &buf[2]); /**/
    addChar(&buf[ret], checkSum_Char); /*270*/ ret +=1;
    addSep(&buf[ret]); /*270*/ ret +=2;
    return ret;
}

int makePlayerReady(char* pointer){
    char* buf = pointer;
    int ret = 0;
    int PN = myClient->PNS;
    char playerID = myClient->playerID;

    addSep(buf); /*2*/ ret +=2;
    addInt(PN, &buf[ret]); /*6*/ ret +=4;
    addChar(&buf[ret], '3'); /* packetID 7*/ ret +=1;
    addInt(1, &buf[ret]); /*packet size 11*/ ret +=4;
    addChar(&buf[ret], playerID); /*12*/ ret +=1;
    char checkSum_Char = checksum( ret-2 , &buf[2]);
    addChar(&buf[ret], checkSum_Char); /*13*/ ret +=1;
    addSep(&buf[ret]); ret +=2;
    return ret;
}

int makePlayerInput(char* pointer, char input){
    char* buf = pointer;
    int PN = myClient->PNS;
    int ret = 0;
    addSep(buf); /*2*/ ret +=2;
    addInt(PN, &buf[ret]); /*6*/ ret +=4;
    addChar(&buf[ret], '8'); /* packetID 6+1=7*/ ret +=1;
    addInt(1, &buf[ret]); /*packet size 7+4=11*/ ret +=4;
    addChar(&buf[ret], input); /*11+1=12*/ ret +=1;
    char checkSum_Char = checksum( ret -2 , &buf[2]);
    addChar(&buf[ret], checkSum_Char); /*12+1=13*/ ret +=1;
    addSep(&buf[ret]); ret +=2;
    return ret;
}

int makeCheckStatus (char* pointer){
    char* buf = pointer;
    int PN = myClient->PNS;

    int ret = 0;
    addSep(buf); /*2*/ ret +=2;
    addInt(PN, &buf[ret]); /*6*/ ret +=4;
    addChar(&buf[6], '9'); /* packetID 7*/ ret +=1;
    addInt(0, &buf[7]); /*packet size 11*/ ret +=4;
    char checkSum_Char = checksum( ret-2 , &buf[2]);
    addChar(&buf[ret], checkSum_Char); /*12+1=13*/ ret +=1;
    addSep(&buf[ret]); ret +=2;
    return ret;
}

char printable_char(char c){
    if(isprint(c) != 0 ) return c;
    return ' ';
}

void print_bytes(void* packet, int count){
    int i;
    unsigned char *p = (unsigned char*) packet;
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
/*
    myClient = malloc(sizeof(struct Client));
    myClient->PN = 0;
    myClient->PNS = 0;
*/

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

    /*
    printf("tiek līdz šejienei");
    fflush(stdout);
    */
   if(connect(my_socket,(struct sockaddr *) &remote_address,sizeof(remote_address)) < 0){
        printf("ERROR connecting\n");
        exit(1);
    }else{
        printf("good connection\n");


        /*shared memory*/
        int sizeofthings = MAXSIZE + sizeof(int) + sizeof(struct Client) + sizeof(struct Object) + sizeof(struct Inputs);
        if (shared_memory = mmap(NULL, sizeofthings, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)){
        payload_size = shared_memory;
        outputs = shared_memory + sizeof(int);
        myClient = outputs + MAXSIZE;
        scr = (struct Object*) (sizeof(myClient) +myClient);
        ball = (struct Object*) (sizeof(scr) +scr);
        player1 = (struct Object*) (sizeof(ball) +ball);
        player2 = (struct Object*) (sizeof(player1) +player1);
        inp = (struct Inputs*) (sizeof(player2) +player2);


        /*initializing objects*/
        *payload_size = 0;
        myClient->PN = 0;
        myClient->PNS = 0;
        }else{
            printf("shared memory allocation for client feiled\n");
        }
        /*shared memory*/


        int pid = fork();
        if (pid == 0){
            gameloop();
        }
        else{
            int pid = fork();
            if (pid == 0){
                writer(my_socket);
            } else {
                reader(my_socket);
            }
        }
    }

    /*tread*/
    /*
    pthread_t tred;
    pthread_create(&tred, NULL, connection_handler, socketname);
    pthread_join(tred, 0);
    */

    return 0;
}
