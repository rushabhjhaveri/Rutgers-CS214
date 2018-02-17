CC = gcc -Wall -Werror -fsanitize=address
RM = rm -f
MEMGRIND = memgrind
OBJECTS = memgrind.o mymalloc.o


$(MEMGRIND): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(MEMGRIND)

memgrind.o: memgrind.c mymalloc.h
	$(CC) -c memgrind.c

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) -c mymalloc.c


clean:
	$(RM) $(MEMGRIND) $(OBJECTS)

