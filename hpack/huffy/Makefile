.PHONY: all check clean

all: bitbuf.o encode.o decode.o

encode-table.c: huffcodes.txt make-etable.sh
	./make-etable.sh < $< > $@

encode.c: encode-table.c

encode.o: encode.c encode.h
	gcc -Wall -g -c -o $@ $<

encode-test: encode-test.c encode.o ../number.c
	gcc -Wall -g -o $@ $^

bitbuf.o: bitbuf.c bitbuf.h
	gcc -Wall -g -c -o $@ $<

bitbuf-test: bitbuf-test.c bitbuf.o
	gcc -Wall -g -o $@ $^

decode.o: decode.c decode.h
	gcc -Wall -g -c -o $@ $<

check: encode-test bitbuf-test
	./encode-test
	./bitbuf-test

clean:
	rm -f encode-test bitbuf-test *.o encode-table.c
