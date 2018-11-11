all: status-code

status-code: status-code.cpp ../StatusStrings.o ArrayStatusStrings.o
	g++ -Wall -O3 -lbenchmark -o $@ $^

ArrayStatusStrings.o: ArrayStatusStrings.cpp ArrayStatusStrings.h
	g++ -Wall -O3 -c -o $@ $<

run: status-code
	./status-code

clean:
	rm -f status-code *.o
