COMPILE = gcc
CFLAGS = -g -Wall

OBJS := test.o varstr.o json.o

all : test
test : ${OBJS}
	${COMPILE} ${CFLAGS} ${OBJS} -o $@

%.o : %.c
	${COMPILE} ${CFLAGS} $< -c -o $@

.PHONY : clean
clean:
	rm *.o *~ test
