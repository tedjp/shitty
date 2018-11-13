all: hello-world counting

CXX = g++ -std=gnu++17 -g -Wall -Werror -fmax-errors=5 -O3 -march=native
#CXX = g++ -std=gnu++17 -g -Wall -Werror -fmax-errors=5 -O0 -march=native
COMPILE_OBJ = $(CXX) -c

%.o: %.cpp %.h
	$(COMPILE_OBJ) -o $@ $<

OBJS = \
	   Connection.o \
	   ConnectionManager.o \
	   CountingResponder.o \
	   Error.o \
	   EventReceiver.o \
	   Headers.o \
	   HTTP1Transport.o \
	   HTTPDate.o \
	   Message.o \
	   Request.o \
	   RequestRouter.o \
	   Server.o \
	   StaticResponder.o \
	   StatusStrings.o \
	   StreamBuf.o \
	   Transport.o \
	   UnhandledRequestHandler.o \
	   #

hello-world: HelloWorld.cpp $(OBJS)
	$(CXX) -I.. -o  $@ $^

counting: CountingServer.cpp $(OBJS)
	$(CXX) -I.. -o $@ $^

clean:
	rm -f *.o hello-world counting

check:
