CC := gcc
CFLAGS=-Wall -Wextra -lm -std=c11 -I ./include/ -Og -g -fno-crossjumping #-DNDEBUG
SRCROOT = .
SRCDIRS := $(shell find $(SRCROOT) -type d)
SRCS=$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))
OBJS=$(SRCS:.c=.o)
TARGET := maxc
.PHONY: test clean

release: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) $(CFLAGS)

run: $(OBJS)
	$(CC) -o $(TARGET) -O3 $(OBJS) $(LDFLAGS) $(CFLAGS)

debug: $(OBJS)
	$(CC) -o $(TARGET) -g -O0 -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

perf: $(OBJS)
	$(CC) -o $(TARGET) -g -Og -DNDEBUG $(OBJS) $(LDFLAGS) $(CFLAGS)

test: $(TARGET)
	sh test/test.sh

clean:
	$(RM) $(OBJS)
