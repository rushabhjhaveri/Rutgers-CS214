CC = gcc
RM = rm -f
MYMALLOC = mymalloc


all: $(MYMALLOC)

$(MYMALLOC): mymalloc.c
	$(CC) -Wall -Werror -o mymalloc mymalloc.c

clean:
	$(RM) $(MYMALLOC)
