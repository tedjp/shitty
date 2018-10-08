all: shitty

run: shitty
	./shitty

%.o: %.cpp %.h
	g++ -std=gnu++11 -Wall -Wextra -Werror -c -o $@ $<

shitty: shitty.cpp error.h settings.h settings.o
	g++ -std=gnu++11 -Wall -Wextra -Werror -o $@ $<

clean:
	rm -f shitty

check:
