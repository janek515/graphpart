TARGET = graphpart
LIBS = -lm
CC = clang
CFLAGS = -g -Wall -Wno-missing-braces --std=gnu23

.PHONY: default all clean

default: ./build/$(TARGET)

all: default

OBJECTS = $(patsubst %.c, ./build/%.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

./build/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

./build/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

# ./build/test: $(OBJECTS)
# 	$(CC) ./build/test_str_util.o ./build/test_point.o ./build/test_leaderboard.o ./build/test_main.o ./build/leaderboard.o ./build/point.o ./build/str_util.o -Wall $(LIBS) -o $@

# test: ./build/test
# 	./build/test

clean:
	-rm -f ./build/*.o
	-rm -f ./build/$(TARGET)

