CC := g++
CFLAGS=-Wall -Wextra
SRCROOT = .
SRCDIRS := $(shell find $(SRCROOT) -type d)
SRCS=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.cpp))
OBJS=$(SRCS:.c=.o)

release: $(OBJS)
	$(CC) -o maxc -O3 -DNDEBUG $(OBJS) $(LDFLAGS)

run: $(OBJS)
	$(CC) -o maxc -O3 -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

debug: $(OBJS)
	$(CC) -o maxc -g -O2 $(OBJS) $(LDFLAGS) $(CFLAGS)

$(OBJS): maxc.h

clean:
	$(RM) *.o
