CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -ggdb
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SOURCES = main.c gui.c simulation.c
TARGET = simulation

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
