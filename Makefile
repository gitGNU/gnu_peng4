DEFS = -DDORKY=0 -DSKIP_XOR=0 -DSKIP_PERMUT=0

CFLAGS = -O2 -Wall -Wno-pointer-sign -std=c90 -ggdb

slowpeng: slowpeng.c slowpeng.h mt19937ar.c mt19937ar.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -o slowpeng slowpeng.c mt19937ar.c

clean:
	rm -f slowpeng *.o core
	rm -f testfile*

test:
	./dobench.sh

