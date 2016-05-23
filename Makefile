CC = gcc

INC = .

FLAGS = -Wall -ansi -pedantic -std=gnu99
OUTPUT = -o
STATIC = -static

RM = rm -f

#--------------------------------

all: client

client: client.o
	$(CC) client.o $(OUTPUT) client $(STATIC)

client.o: client.c client.h
	$(CC) $(FLAGS) -I$(INC) -c client.c

clean:
	-@$(RM) client.o client

run:
	@echo "Running program...";
	@./client
