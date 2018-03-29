CC = gcc -Wall -Werror
INDEX = index


all: $(INDEX)

$(INDEX): index.o strArr2.o
		$(CC) -o index index.o strArr2.o

$(INDEX).o: index.c index.h
		$(CC) -c index.c index.h

strArr2.o: strArr2.c strArr2.h
		$(CC) -c strArr2.c strArr2.h

clean:
		-rm -f *.o
		-rm -f $(INDEX)
		-rm -f *.h.gch
