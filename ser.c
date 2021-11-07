struct sockaddr_in remote_adress;
char host[40];
int port;


void* connectToServer(void* clientSocket){
    int second = 1000;
    char inputs[100];
    int my_socket = (int)clientSocket;
    free(clientSocket);
    if(connect(my_socket,(struct sockaddr *)&remote_adress,sizeof(remote_adress))<0){
        printf("ERROR WHEN MAKING CONNECTION\n");
        exit(1);
    }
    else{
        char recievedLine[MAX_LINE];
        memset(recievedLine,0,MAX_LINE);
        while(1){
            scanf("%s",inputs);
            send(my_socket,inputs,strlen(inputs),0);
            sleep(0.1);
            if(read(my_socket,recievedLine,MAX_LINE-1)>0) printf("%s",recievedLine);
        }
    }
    return NULL;
}


int main(int argc, char* argv[]){
    char* argument = argv[1];
    char* argument1= argv[2];
    char* onlyArg;
    char* onlyArg1;
    if(strncmp(argument,IP_CHECK,2)!=0&&strncmp(argument,PORT_CHECK,2)!=0){
        printf("First input paramater is incorrect.\n");
        return -1;
    }
    else if(strncmp(argument,IP_CHECK,2)!=0&&strncmp(argument,PORT_CHECK,2)!=0){
        printf("Second input paramater is incorrect.\n");
        return -1;
    }
    int j;
    for(j=0;j<2;j++) onlyArg = strsep(&argument,"=");
    for(j=0;j<2;j++) onlyArg1 = strsep(&argument1,"=");

    if(strncmp(argv[1],IP_CHECK,3)==0){
        strcpy(host,onlyArg);
        port = atoi(onlyArg1);
    }
    else{
        strcpy(host,onlyArg1);
        port = atoi(onlyArg);
    }

    int my_socket = 0;
    char* servername;

    remote_adress.sin_family = AF_INET;
    remote_adress.sin_port = htons(port);
    servername =gethostbyname(host);
    inet_pton(AF_INET,servername,&remote_adress.sin_addr);

    if((my_socket = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("SOCKET ERROR\n");
        return -1;
    }

    int *socketAdress = malloc(sizeof(int));
    *socketAdress = my_socket;
    pthread_t thread;
    pthread_create(&thread,NULL,connectToServer,socketAdress);
    pthread_join(thread, 0);



    return 0;
}