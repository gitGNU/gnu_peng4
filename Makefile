DEFS = -DDORKY=0 -DSKIP_XOR=0 -DSKIP_PERMUT=0 -DDEBUG=0

CFLAGS = -O2 -Wall -Wno-pointer-sign -std=c90 -ggdb

slowpeng: slowpeng.c slowpeng.h mt19937ar.c mt19937ar.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -o slowpeng slowpeng.c mt19937ar.c

countbits: countbits.c
	$(CC) $(CFLAGS) -o countbits countbits.c

clean:
	rm -f slowpeng *.o core countbits
	rm -f testfile*

test:
	./dobench.sh

