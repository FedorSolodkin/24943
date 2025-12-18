server30: server30.c
	gcc -o server30 server30.c -lsocket -lnsl

client30: client30.c
	gcc -o client30 client30.c -lsocket -lnsl

run: server30 client30
	./server30 & sleep 1; ./client30
