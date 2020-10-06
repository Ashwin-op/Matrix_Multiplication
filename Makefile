CC=gcc
CFLAGS=-c -Wall -g
SOURCES=MatrixMultiplication.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=MatrixMult

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)