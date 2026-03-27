CC     = gcc
CFLAGS = -Wall -Wextra -g
LIBS   = -lreadline -lm

ojcsh: main.o parser.o executor.o builtins.o
	$(CC) $(CFLAGS) -o ojcsh $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o ojcsh
