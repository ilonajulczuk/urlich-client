CC=g++
CFLAGS=-L/usr/local/lib -ljson-c -L/usr/lib/x86_64-linux-gnu -lcurl
INCLUDES=-I/usr/local/include/json-c/

all: hello

hello: main.o
	$(CC) main.o -o hello $(CFLAGS)

main.o: main.cpp
	$(CC) -Wall -c main.cpp -o main.o $(INCLUDES)

clean:
	rm *o hello
