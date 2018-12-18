CC=gcc
CFLAGS=-pthread -I. -Wall -Wno-int-conversion -D_GNU_SOURCE

binaries=signal signal_nostatevar

all: $(binaries)

pthread_mult: pthread.c pthread_int.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) -f $(binaries) *.o
    