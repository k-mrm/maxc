CC := gcc
CFLAGS=-Wall -Wextra -std=c11 -g -DNDEBUG -Og
SRCROOT = .
SRCDIRS := $(shell find $(SRCROOT) -type d)
SRCS=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
OBJS=$(SRCS:.c=.o)

release: $(OBJS)
	$(CC) -o maxc -O3 -DNDEBUG $(OBJS) $(LDFLAGS)

run: $(OBJS)
	$(CC) -o maxc -O3 $(OBJS) $(LDFLAGS) $(CFLAGS)

debug: $(OBJS)
	$(CC) -o maxc -g -O0 -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

perf: $(OBJS)
	$(CC) -o maxc -g -Og -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

$(OBJS): src/maxc.h

clean:
	$(RM) src/*.o
