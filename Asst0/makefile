CC = gcc
RM = rm -f
STRINGSORTER = stringsorter


all: $(STRINGSORTER)

$(STRINGSORTER): stringsorter.c
	$(CC) -Wall -Werror -o stringsorter stringsorter.c

$(FORMAT): format.c

clean:
	$(RM) $(STRINGSORTER)
