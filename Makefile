CC=g++
CFLAGS=-Wall
CFLAGS += -L/usr/local/lib -ljson-c -L/usr/lib/x86_64-linux-gnu -lcurl -I/usr/local/include/json-c/

all: hello

hello: main.o
	$(CC) main.o -o hello

main.o: main.cpp
	$(CC) main.cpp -o main.o $(CFLAGS) 

clean:
	rm *o hello
