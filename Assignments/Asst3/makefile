GCC = gcc -Wall -Werror
RM = rm -f
CLIENT = client
SERVER = server

all: $(CLIENT) $(SERVER)


$(CLIENT): libnetfiles.c
			$(GCC) -o client libnetfiles.c

$(SERVER): netfileserver.c
			$(GCC) -pthread -o server netfileserver.c

clean:
			$(RM) $(CLIENT)
			$(RM) $(SERVER)
