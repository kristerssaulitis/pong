all: server_2.0  client

run1: server_2
run2: client2

server_2.0: server_2.0.o
	gcc -Wall -std=gnu11 server_2.0.o -o server_2.0.exe

client: client.o
	gcc -Wall -std=gnu11 client.o -o client.exe -lpthread

server_2.0.o: server_2.0.c
	gcc -Wall -std=gnu11 -c server_2.0.c

client.o: client.c
	gcc -Wall -std=gnu11 -c client.c

clean:
	rm *.o *.exe

server_2: ./server_2.0.exe
	./server_2.0.exe -p=12566

client2: ./client.exe
	./client.exe -a=localhost -p=12566
