CC := gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
TARGET := snake.exe
SOURCES := snake.c user.c win.c
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all build run test clean

all: build

build: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o: %.c snake.h user.h win.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) --self-test

clean:
	powershell -NoProfile -Command "Remove-Item -Path snake.o,user.o,win.o,snake.exe -Force -ErrorAction SilentlyContinue; exit 0"
