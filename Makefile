all: shitty
run: shitty
	./shitty
shitty: shitty.c
	gcc -std=gnu99 -Wall -Wextra -Werror -o $@ $<
clean:
	rm -f shitty
check:
