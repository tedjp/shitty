all: shitty

run: shitty
	./shitty

%.o: %.cpp %.h
	g++ -std=gnu++11 -Wall -Wextra -Werror -c -o $@ $<

OBJS = settings.o socket.o server.o error.o

shitty: shitty.cpp $(OBJS)
	g++ -std=gnu++11 -Wall -Wextra -Werror -o $@ $^

clean:
	rm -f shitty

check:
