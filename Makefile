DEFS = -DDORKY=0 -DSKIP_XOR=0 -DSKIP_PERMUT=0 -DDEBUG=0 -DBETA=1 -DALPHA=1 \
 -DUSE_MODE_XPX=1 -DUSE_MODE_CBC=1

CFLAGS = -O2 -Wall -Wno-pointer-sign -std=c90 -ggdb -D_GNU_SOURCE

SOURCES= peng_ref.c whirlpool.c mt19937ar.c peng.c peng_misc.c

TARGETS= $(addsuffix .o, $(basename $(SOURCES)))

all: peng countbits

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFS) $< -o $@

peng: $(TARGETS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -o peng $(TARGETS)

countbits: countbits.c
	$(CC) $(CFLAGS) -o countbits countbits.c

clean:
	rm -f peng peng_ref *.o core countbits
	rm -f testfile* *~

test:
	./dobench.sh

##############################################################################

ci: clean
	./updver.py
	git add *.c *.h Makefile *.sh TODO README LICENSE *.py
	git ci

keywords:
	git-keyw-filter *.cc *.hh Makefile

push:
	git push -u origin master

##############################################################################

tarball:
	tar -cjf peng4-$(shell ./getver.sh).tar.bz2 *

