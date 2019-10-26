CC := gcc
CFLAGS=-Wall -Wextra -std=c11 -I ./include/ -Os
SRCROOT = .
SRCDIRS := $(shell find $(SRCROOT) -type d)
SRCS=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
OBJS=$(SRCS:.c=.o)
.PHONY: clean

release: $(OBJS)
	$(CC) -o maxc $(OBJS) $(LDFLAGS)

run: $(OBJS)
	$(CC) -o maxc -O3 $(OBJS) $(LDFLAGS) $(CFLAGS)

debug: $(OBJS)
	$(CC) -o maxc -g -O0 -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

perf: $(OBJS)
	$(CC) -o maxc -g -Og -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

clean:
	$(RM) src/*.o
	$(RM) src/vm/*.o
	$(RM) src/compiler/*.o
	$(RM) src/error/*.o
