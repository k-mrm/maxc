CC := g++
CFLAGS=-Wall -Wextra
SRCROOT = .
SRCDIRS := $(shell find $(SRCROOT) -type d)
SRCS=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.cpp))
OBJS=$(SRCS:.c=.o)

release: $(OBJS)
	$(CC) -o maxc -O3 $(OBJS) $(LDFLAGS)

run: $(OBJS)
	$(CC) -o maxc -O3 $(OBJS) $(LDFLAGS) $(CFLAGS)

debug: $(OBJS)
	$(CC) -o maxc -g -O0 -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

perf: $(OBJS)
	$(CC) -o maxc -g -O2 -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

$(OBJS): src/maxc.h

clean:
	$(RM) *.o
