CC = gcc
CFLAGS = -g #-Wextra -Wall
files = main.c bitmap.c
output = tile-builder

$(output) : $(files)
	$(CC) $(CFLAGS) -o $(output) $(files)

PHONY : clean
clean :
	rm $(output)
