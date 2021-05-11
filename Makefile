all: \
	hello-world \
	print-requests \
	proxy \
	#

#CXX = g++ -std=gnu++17 -g -Wall -Werror -fmax-errors=5 -O3 -march=native
CXX = g++ -std=c++20 -g -Wall -Werror -fmax-errors=1 -O0 -march=native
COMPILE_OBJ = $(CXX) -c

%.o: %.cpp %.h
	$(COMPILE_OBJ) -o $@ $<

OBJS = \
	   Connection.o \
	   ConnectionManager.o \
	   Date.o \
	   Error.o \
	   EventReceiver.o \
	   Headers.o \
	   http1/ClientTransport.o \
	   http1/ClientTransportSource.o \
	   http1/HTTP1.o \
	   http1/HTTP2Upgrader.o \
	   http1/ServerTransport.o \
	   http1/Transport.o \
	   http1/UpgradeRegistry.o \
	   http2/ServerStream.o \
	   http2/ServerTransport.o \
	   Message.o \
	   ProxyHandler.o \
	   Request.o \
	   RequestHandler.o \
	   Response.o \
	   Route.o \
	   Routes.o \
	   Server.o \
	   Stream.o \
	   SignalReceiver.o \
	   SignalSource.o \
	   StaticResponder.o \
	   StatusStrings.o \
	   StreamBuf.o \
	   UnhandledRequestHandler.o \
	   #

LDLIBS = \
		 -lfb64 \
		 #

hello-world: HelloWorld.cpp $(OBJS)
	$(CXX) -I.. -o  $@ $^ $(LDLIBS)

proxy: ProxyServer.cpp $(OBJS)
	$(CXX) -I.. -o $@ $^ $(LDLIBS)

print-requests: PrintRequestServer.cpp $(OBJS)
	$(CXX) -o $@ $^ $(LDLIBS)

clean:
	rm -f \
		*.o \
		http1/*.o \
		http2/*.o \
		hello-world \
		print-requests \
		proxy \
		#

check:
