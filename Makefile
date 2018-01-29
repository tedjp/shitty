all: shitty
run: shitty
	./shitty
shitty: shitty.c
	gcc -Wall -o shitty shitty.c
clean:
	rm -f shitty
