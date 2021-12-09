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

int main () {

}

unsigned char * makeJoin(char* Username){
    int PN = 1;
    unsigned char* buf;

    addSep(buf);
    addInt(PN, &buf[2]);
    addInt(1, &buf[6]);
    addLong(20, &buf[10]);
    char checkSum_Char = checksum( 20, Username);
    add_string(&checkSum_Char, &buf[18], 1);
    add_string(Username, &buf[19], strlen(Username));
    addSep(&buf[39]);
    return buf;
}

int getPacketNumber(unsigned char* packet){
    int val = 0;

    int j = 0;
    int i;
    for ( i = 3; i >=0; --i)
    {
        val += (packet[i] & 0xFF) << (8*j);
        ++j;
    }

    return val;
}

void addSep(unsigned char * buf){
    buf[0] = '-';
    buf[1] = '-';
}

void addInt(int num, unsigned char * buf){
    int i = 0;
    for(i = 0; i < sizeof(int); i++){
        buf[i] = (num >> (sizeof(int)-1-i)*8) & 0xff;
    }
}

void addLong(long num, unsigned char * buf){
    int i = 0;
    buf[0] = num & 0xff;
    for(i = 0; i < sizeof(long); i++){
        buf[i] = (num >> (sizeof(long)-1-i)*8) & 0xff;
    }
}

void add_string(char* str, unsigned char* buf, int count){
    int i = 0;
    for(i = 0; i < count; i++){
        buf[i] = str[i];
    }
}

char checksum(int length, char* packet){
    char checksum = 0;
    for(int i=0; i<length; i++){
        checksum ^= packet[i];
    }
    return checksum;
}








unsigned char * makeJoin(unsigned char* pointer, char* Username){
    
    unsigned char* buffer = pointer;

    addSeperator(buffer);
    addInt(PN, &buffer[2]);
    addInt(1, &buffer[6]);
    addLong(20, &buffer[10]);
    char checkSumChar = calculateCheckSum(Username, 20);
    addString(&checkSumChar, &buffer[18], 1);
    addString(Username, &buffer[19], strlen(Username));
    addSeperator(&buffer[39]);
    return buffer;
}

while(1){
        read(socket, in, 1);
        printf("šis ir for sientific purpouses only %c, this is how many bites were were %li", in[0], read(socket, in, 1));
        fflush(stdout);
        while((rc = recv(newsockfd,pkt,sizeof(packet_size),0)) > 0){
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


    int rc =0;
/*-- nfdnvakjnfvjkdnfbkanf -- */
/*šeit no iepriekšājā paliek -- abcdejdirst, šeit no nākamā sākas -- */
/*-- nfdnvakjnfvjkdnfbkanf -- */
        while((rc = recv(socket,in,1,0)) > 0){
            printf("this is recv: %s", in);
            fflush(stdout);
            if (inpacket == 0){
                if(in[0]=='-'){
                    read(socket, in, 1);
                    if(in[0]=='-'){
                        ++sepCounter;
                        printf("\n do you even listen 5 .0 %c \n ", in[0]);
                            if (sepCounter==2){
                                sepCounter = 0;
                                inpacket = 1;

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