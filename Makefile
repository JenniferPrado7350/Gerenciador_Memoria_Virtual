CC=gcc
CFLAGS=-Wall

GerenciadorMemoriaVirtual: GerenciadorMemoriaVirtual.o GerenciadorMemoriaVirtual.c
	$(CC) $(CFLAGS) -o GerenciadorMemoriaVirtual GerenciadorMemoriaVirtual.o

GerenciadorMemoriaVirtual.o: GerenciadorMemoriaVirtual.c
	$(CC) $(CFLAGS) -c GerenciadorMemoriaVirtual.c

clean:
	rm -f *.o GerenciadorMemoriaVirtual.exe GerenciadorMemoriaVirtual
