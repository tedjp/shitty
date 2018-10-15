all: wtfd2 shitty

run: shitty
	./shitty

CXX = g++ -std=gnu++17 -Wall -Wextra -Werror -fmax-errors=10
COMPILE_OBJ = $(CXX) -c

%.o: %.cpp %.h
	$(COMPILE_OBJ) -o $@ $<

OBJS = settings.o socket.o server.o error.o tcpserver.o

shitty: shitty.cpp $(OBJS)
	$(CXX) -o $@ $^

wtfd2: wtfd2.cpp $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm -f shitty wtfd2 *.o

check:
