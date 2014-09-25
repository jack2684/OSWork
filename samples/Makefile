.SUFFIXES: .c

SRCS = demo.c
OBJS = $(SRCS:.c=.o)
OUTPUT = demo

CC = gcc
CFLAGS = 
LIBS = 

demo: $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $(OUTPUT)

depend:
	makedepend -I/usr/local/include/g++ -- $(CFLAGS) -- $(SRCS) 

# DO NOT DELETE


