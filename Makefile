CFLAGS = -O2 -Wall -Wno-pointer-sign -std=c90 -ggdb

slowpeng: slowpeng.c slowpeng.h mt19937-64.c mt19937-64.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o slowpeng slowpeng.c mt19937-64.c

clean:
	rm -f slowpeng *.o
