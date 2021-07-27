CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lmosquitto -lsqlite3 -luci 

TARGET = msqt
DEPS = $(wildcard *.h)
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: clean

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(OBJS) -f $(TARGET)