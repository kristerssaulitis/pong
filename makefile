s:
runs:
c:
	gcc -Wall client.c -std=c99 -o client.exe
runc:
	./client.exe client -a=localhost -p=12345