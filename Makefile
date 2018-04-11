all: shitty
run: shitty
	./shitty
shitty: shitty.c
	gcc -Wall -Wextra -Werror -o shitty shitty.c
clean:
	rm -f shitty
