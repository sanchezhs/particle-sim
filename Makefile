CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

raygui:
	$(CC) $(CFLAGS) -o controls controls.c $(LDFLAGS)


main:
	$(CC) $(CFLAGS) -o main main.c $(LDFLAGS)

clean:
	rm -f main