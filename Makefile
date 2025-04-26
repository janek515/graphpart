TARGET = graphpart
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall -Wno-missing-braces

.PHONY: default all clean

default: dirs ./build/$(TARGET)

all: default

# Create build directory if it doesn't exist
dirs:
	mkdir -p ./build

# Change source file path to src directory
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, ./build/%.o, $(SOURCES))
HEADERS = $(wildcard src/*.h)

# Update the object file compilation rule
./build/%.o: src/%.c $(HEADERS)
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
	-rmdir ./build
