CC = gcc
CFLAGS = -g -std=c99 -Wall -O2
LIBS = -lgslcblas -lgsl -lm
LDFLAGS = $(LIBS)
OBJS = generator.o
SRCS = $(OBJS:.o=.c)
TARGET = generator

all: $(TARGET)

Makefile.dep dep:
	$(CC) -MM $(CFLAGS) $(SRCS) > Makefile.dep

$(TARGET): $(OBJS)

include Makefile.dep

clean:
	rm -f $(OBJS) $(TARGET)

clean-all: clean
	rm -f process_info Makefile.dep
