CC := g++
CFLAGS=-Wall -Wextra -O3
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.c=.o)

run: $(OBJS)
	$(CC) -o maxc $(OBJS) $(LDFLAGS) $(CFLAGS)

debug: $(OBJS)
	$(CC) -o maxc -g $(OBJS) $(LDFLAGS) $(CFLAGS)

$(OBJS): maxc.h

clean:
	$(RM) *.o
