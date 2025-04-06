TARGET = graphpart
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall -Wno-missing-braces

.PHONY: default all clean

default: $(TARGET)

all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

# ./build/test: $(OBJECTS)
# 	$(CC) ./build/test_str_util.o ./build/test_point.o ./build/test_leaderboard.o ./build/test_main.o ./build/leaderboard.o ./build/point.o ./build/str_util.o -Wall $(LIBS) -o $@

# test: ./build/test
# 	./build/test

clean:
	-rm -f *.o
	-rm -f $(TARGET)

