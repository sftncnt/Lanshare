SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

lanshare: $(OBJ)
	gcc -Wall -Wextra -O2 -o lanshare $(OBJ) -lcrypto
