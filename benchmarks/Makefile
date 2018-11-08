all: status-code

status-code: status-code.cpp ../StatusStrings.o
	g++ -Wall -O3 -lbenchmark -o $@ $^

run: status-code
	./status-code

clean:
	rm -f status-code
