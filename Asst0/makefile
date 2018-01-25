CC = gcc
CCFLAGS = -g -Wall -lm -I.
RM = rm -f
STRINGSORTER = stringsorter


all: $(STRINGSORTER)

$(STRINGSORTER): stringsorter.c
	$(CC) $(CCFLAGS) -o stringsorter stringsorter.c

$(FORMAT): format.c
	
clean:
	$(RM) $(STRINGSORTER)
