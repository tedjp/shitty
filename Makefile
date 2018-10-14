all: shitty

run: shitty
	./shitty

%.o: %.cpp %.h
	g++ -std=gnu++17 -Wall -Wextra -Werror -fmax-errors=10 -c -o $@ $<

OBJS = settings.o socket.o server.o error.o tcpserver.o

shitty: shitty.cpp $(OBJS)
	g++ -std=gnu++17 -Wall -Wextra -Werror -fmax-errors=10 -o $@ $^

clean:
	rm -f shitty

check:
