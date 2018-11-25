all: hello-world counting per-request proxy

CXX = g++ -std=gnu++17 -g -Wall -Werror -fmax-errors=5 -O3 -march=native
#CXX = g++ -std=gnu++17 -g -Wall -Werror -fmax-errors=5 -O0 -march=native
COMPILE_OBJ = $(CXX) -c

%.o: %.cpp %.h
	$(COMPILE_OBJ) -o $@ $<

OBJS = \
	   ClientTransportSource.o \
	   Connection.o \
	   ConnectionManager.o \
	   Date.o \
	   Error.o \
	   EventReceiver.o \
	   Headers.o \
	   http1/ClientTransport.o \
	   http1/HTTP1.o \
	   http1/ServerTransport.o \
	   http1/Transport.o \
	   Message.o \
	   ProxyHandler.o \
	   Request.o \
	   RequestHandler.o \
	   RequestRouter.o \
	   Response.o \
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

per-request: PerRequestServer.cpp $(OBJS)
	$(CXX) -I.. -o $@ $^

proxy: ProxyServer.cpp $(OBJS)
	$(CXX) -I.. -o $@ $^

clean:
	rm -f *.o http1/*.o hello-world counting per-request proxy

check:
