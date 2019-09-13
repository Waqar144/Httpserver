CFLAGS = -g -Wall

all: server

server: utility_funcs.o

clean:
	rm -f *.o
	rm -f server