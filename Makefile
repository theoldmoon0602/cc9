cc9: cc9.c

test: cc9
	./test.sh

clean:
	rm -f cc9 *.o tmp*
