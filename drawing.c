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

void gameloop();
void circle(float x, float y, float r);

int main(){
    initscr();
    noecho();
    curs_set(FALSE);
    gameloop();
    endwin();
    return 0;
}

void gameloop(){
    while(1){
        circle(1.0, 1.0, 3.0);
    }

}

void circle(float x, float y, float r){
    mvprintw(y, x, "o");
}