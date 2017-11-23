CC=cc
CFLAGS=-Wall
CFLAGS+=-std=c11
LIBS=$(shell pkg-config --cflags --libs rdkafka)

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

all: kafka_test kafka_prep

kafka_test: $(OBJ)
	$(CC) $(LIBS) -o $@ $^

clean:
	rm -f kafka_test
	rm -f *.o

kafka_prep:
	./init_kafka.sh