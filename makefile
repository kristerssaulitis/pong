all: server_2.0  client

run: server_2.0.run  client.run

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

server_2.0.run:
	./server_2.0 -p=12345

client.run:
	./client -a=localhost -p=12345