CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGETS = publicadorp suscriptorp_modificado sistemap_modificado

all: $(TARGETS)

publicadorp: publicadorp.c
	$(CC) $(CFLAGS) -o publicador publicadorp.c

suscriptorp_modificado: suscriptorp_modificado.c
	$(CC) $(CFLAGS) -o suscriptor suscriptorp_modificado.c

sistemap_modificado: sistemap_modificado.c
	$(CC) $(CFLAGS) -o sistema sistemap_modificado.c

clean:
	rm -f $(TARGETS)
