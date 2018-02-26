.PHONY: check all

encode-table.c: huffcodes.txt make-etable.sh
	./make-etable.sh < $< > $@

test: test.c encode.c ../number.c
	gcc -Wall -g -o $@ $^

check: test
	./test
