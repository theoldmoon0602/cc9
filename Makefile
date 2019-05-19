CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc9: $(OBJS)
	$(CC) -o cc9 $(OBJS) $(LDFLAGS)

$(OBJS): cc9.h


test: cc9
	./cc9 -test
	./test.sh

clean:
	rm -f cc9 *.o tmp*
