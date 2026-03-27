CC     = gcc
CFLAGS = -Wall -Wextra -g
LIBS   = -lreadline -lm

ojc: main.o parser.o executor.o builtins.o
	$(CC) $(CFLAGS) -o ojc $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o ojc
