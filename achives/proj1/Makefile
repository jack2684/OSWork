.SUFFIXES: .c

SRCS = src/album.c
OBJS = $(SRCS:.c=.o)
OUTPUT = album

CC = gcc
CFLAGS = -g -lpthread -m32
LIBS = 

demo: $(OBJS)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $(OUTPUT)

depend:
	makedepend -I/usr/local/include/g++ -- $(CFLAGS) -- $(SRCS) 

# DO NOT DELETE
#
#
#
