# This doesn't work with `make -j` because `make depend` needs to complete
# before other things. makedepend(1) might help. For now, the recommendation
# is to run `make depend` once before anything else.
all: \
	depend \
	hello-world \
	print-requests \
	proxy \
	#

depend:
	$(MAKE) -C dependencies/fb64 -j

CXX = g++ -std=c++20 -pipe -g -Wall -Werror -fmax-errors=1 -O0 -march=native
CPPFLAGS = -Idependencies/fb64
COMPILE_OBJ = $(CXX) $(CPPFLAGS) -c

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
	   http2/Frame.o \
	   http2/ServerStream.o \
	   http2/ServerTransport.o \
	   http2/Settings.o \
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
		 -Ldependencies/fb64 -lfb64 \
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
	$(MAKE) -C dependencies/fb64 clean

check:
	$(MAKE) -C dependencies/fb64 check

.PHONY: all check clean depend
