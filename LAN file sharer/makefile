CC=gcc
CFLAGS = -Wall -Wextra -std=gnu11
OBJ=main.o network.o utils.o transfer.o

lanshare: $(OBJ)
	$(CC) $(CFLAGS) -o lanshare $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o lanshare