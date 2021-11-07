all: server  client

server: hw1.o server.o
	gcc -Wall -std=gnu90 server.o hw1.o -o server.exe

client: hw1.o client.o
	gcc -Wall -std=gnu90 client.o hw1.o -o client.exe -lpthread

server.o: server.c
	gcc -Wall -std=gnu90 -c server.c

client.o: client.c
	gcc -Wall -std=gnu90 -c client.c

hw1.o: hw1.c  hw1.h
	gcc -Wall -std=gnu90 -c hw1.c

clean:
	rm *.o *.exe
