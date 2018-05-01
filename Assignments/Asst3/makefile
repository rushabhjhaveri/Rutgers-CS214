GCC = gcc -Wall -Werror
RM = rm -f
CLIENT = client
CLIENT1 = client1
CLIENT2 = client2
SERVER = server

all: $(CLIENT2) $(CLIENT1) $(CLIENT) $(SERVER)

$(CLIENT2): libnetfiles.c client2.c
			$(GCC) -pthread -o client2 libnetfiles.c client2.c

$(CLIENT1): libnetfiles.c client1.c
			$(GCC) -pthread -o client1 libnetfiles.c client1.c
$(CLIENT): libnetfiles.c client.c
			$(GCC) -pthread -o client libnetfiles.c client.c

$(SERVER): netfileserver.c
			$(GCC) -pthread -o server netfileserver.c

clean:
			$(RM) $(CLIENT)
			$(RM) $(SERVER)
			$(RM) $(CLIENT1)
			$(RM) $(CLIENT2)
