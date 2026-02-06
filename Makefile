# Roll No: MT25086
# File: MT25086_Makefile

CC = gcc
CFLAGS = -pthread -Wall -O2
DEPS = MT25086_Part_A_Common.h

all: server_a1 client_a1 server_a2 server_a3

server_a1: MT25086_Part_A1_Server.c $(DEPS)
	$(CC) $(CFLAGS) -o MT25086_Part_A1_Server MT25086_Part_A1_Server.c

client_a1: MT25086_Part_A1_Client.c $(DEPS)
	$(CC) $(CFLAGS) -o MT25086_Part_A1_Client MT25086_Part_A1_Client.c

server_a2: MT25086_Part_A2_Server.c $(DEPS)
	$(CC) $(CFLAGS) -o MT25086_Part_A2_Server MT25086_Part_A2_Server.c

server_a3: MT25086_Part_A3_Server.c $(DEPS)
	$(CC) $(CFLAGS) -o MT25086_Part_A3_Server MT25086_Part_A3_Server.c

clean:
	rm -f MT25086_Part_A1_Server MT25086_Part_A1_Client MT25086_Part_A2_Server MT25086_Part_A3_Server