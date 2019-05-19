cc9: cc9.c

test: cc9
	./cc9 -test
	./test.sh

clean:
	rm -f cc9 *.o tmp*
