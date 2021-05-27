tests:
	gcc -Iinclude -Itest -DLOGEX_LOCAL -ggdb test/test.c test/test-other-file.c -o logex-test

install:
	cp include/* /usr/local/include

clean:
	rm -f logex-test
	rm -f *.o
	rm -f *.i
	rm -f *.s
