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
struct Client *myClient = NULL;
/*for client to understand at which state it is*/


struct Client {
    unsigned int PN;
    char name[20];
    char playerID;
    char targetID;
    char gameType;
    char teamID;
    char ready;
    char message[256];

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
/*----------------------------------*/

void *connection_handler(void* socket_desc);
void addSep( char * buf);
void addInt(int num,  char * buf);
void addLong(long num,  char * buf);
void add_string(char* str,  char* buf, int count);
char checksum(int length, char* packet);
int makeJoin( char* pointer);
int makePlayerInput(char* pointer, char input);

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
        printf("old packet recieved\n");
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
    /*printf("valid packet\n");*/


    /*printing*/
/*
    print_bytes(out, size + 10);
    printf("our id is %c", ID);
*/
    if(ID == '2'){
        printf("unknown packet recieved 2\n");
    }else if(ID == '3'){
        printf("unknown packet recieved 3\n");
    }else if(ID == '7'){
        printf("unknown packet recieved 7\n");
    }else if(ID == '5'){
        printf("unknown packet recieved 5\n");
    }else if(ID == '4'){
        printf("unknown packet recieved 4\n");
    }else if (ID == '10'){
        printf("unknown packet recieved 10\n");
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
        /*char buffer[1240];
        if (read(socket, buffer, 15)>0) print_bytes(buffer, 15);
        fflush(stdout);
        char out[1000];
        */
        char out[1000];
        while (read(my_sock, in, 1)){
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
                            print_bytes(out, 11);
                            unwrapping(out);
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
    char outputs[MAXSIZE];
    while(1){
        /*for sending from client to server*/
        int payload_size = 0;
        strcpy(myClient->name,"yo");
        myClient->PN += 1;
        payload_size = makeJoin(outputs);

        /*sitos spagetus lugums apiet ar likumu - tadu jobanumu es vel nebiju ieprieks rakstijis*/
            /*escaping packet*/
        int ue;
        int es_size = 0;
        for(ue = 2; ue < payload_size - 2; ue++){
                if(outputs[ue] == '?'){
                        int i = ue + 1;
                        char temp1;
                        char temp2;
                        for(i; i <= payload_size;i++){
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
                        for(i; i <= payload_size;i++){
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

        send(my_sock,outputs, payload_size + es_size,0);
        memset(outputs, 0, payload_size +es_size);
        es_size = 0;


        usleep(1000* 200);
    }
    return;
    }

/*
void *connection_handler(void* args){

    int my_sock = *(int*) args;
    free(args);

    if(connect(my_sock,(struct sockaddr *) &remote_address,sizeof(remote_address)) < 0){
        printf("ERROR connecting\n");
        exit(1);
    }else{
        printf("good connection\n");
    }
    return NULL;
}
*/



/*Packet functions*/


/*Note although I coppied here 10 functions the actual count for client is less some functions are used by client but some by server so it will change*/

int makeJoin(char* pointer ){
    char* buf = pointer;
    int PN = myClient->PN;
    char name[20];
    strcpy(name,myClient->name);

    addSep(buf);
    addInt(PN, (char*)buf+2);
    addChar(&buf[6], '1'); /* packetID*/
    addInt(20, &buf[7]); /*packet size*/
    add_string(name, &buf[11], 20);
    char checkSum_Char = checksum(29, &buf[2]);
    addChar(&buf[31], checkSum_Char);
    addSep(&buf[32]);
    return 34;
}



int makeMessage(char* pointer ){
    char* buf = pointer;
    int PN = myClient->PN;
    char message[256];
    strcpy(message,myClient->name);
    char playerID = myClient->playerID;

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

int makePlayerReady(char* pointer){
    char* buf = pointer;

    int PN = myClient->PN;
    char playerID = myClient->playerID;

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
    int PN = myClient->PN;

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
    int PN = myClient->PN;

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
    myClient = malloc(sizeof(struct Client));
    myClient->PN = 0;
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
        int pid = fork();
        if (pid == 0){
            reader(my_socket);
        }
        else{
            writer(my_socket);
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
