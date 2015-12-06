CC=g++
# Includes are needed for compilation, compiler will recognize signatures.
INCLUDES=-I/usr/local/include/json-c
# To perform linking we need to know what we want to link,
# and where are those libraries.
LFLAGS=-L/usr/local/lib -ljson-c -L/usr/lib/x86_64-linux-gnu -lcurl

all: hello

hello: main.o
	$(CC) main.o -o hello $(LFLAGS)

main.o: main.cpp
	$(CC) -Wall -c main.cpp -o main.o $(INCLUDES)

clean:
	rm *.o hello
