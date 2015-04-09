DEFS = -DDORKY=0 -DSKIP_XOR=0 -DSKIP_PERMUT=0 -DDEBUG=0

CFLAGS = -O2 -Wall -Wno-pointer-sign -std=c90 -ggdb

SOURCES= slowpeng.c whirlpool.c mt19937ar.c peng.c

TARGETS= $(addsuffix .o, $(basename $(SOURCES)))

all: peng countbits

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFS) $< -o $@

peng: $(TARGETS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -o peng $(TARGETS)

countbits: countbits.c
	$(CC) $(CFLAGS) -o countbits countbits.c

clean:
	rm -f peng slowpeng *.o core countbits
	rm -f testfile*

test:
	./dobench.sh

##############################################################################

ci: clean
	git add *.c *.h Makefile *.sh TODO
	git ci

keywords:
	git-keyw-filter *.cc *.hh Makefile

push:
	git push -u origin master

##############################################################################
