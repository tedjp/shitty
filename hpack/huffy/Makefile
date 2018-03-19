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

decode-test: decode-test.c decode.o bitbuf.o
	gcc -Wall -g -o $@ $^

random-test: random-test.c encode.o decode.o bitbuf.o ../number.c
	gcc -Wall -g -o $@ $^

TESTS = bitbuf-test encode-test decode-test random-test
check: $(TESTS)
	set -e; \
	for t in $(TESTS); do \
		echo Executing $$t; \
		./$$t; \
	done

clean:
	rm -f $(TESTS) *.o encode-table.c
