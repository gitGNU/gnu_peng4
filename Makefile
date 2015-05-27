DEFS = -DDORKINESS=0 -DSKIP_XOR=0 -DSKIP_PERMUT=0 -DDEBUG=0 -DBETA=1 -DALPHA=0 \
 -DUSE_MODE_XPX=1 -DUSE_MODE_CBC=1 -DLINUX=1 -DTESTARCH=0

CFLAGS = -O3 -Wall -Wno-pointer-sign -std=c90 -D_GNU_SOURCE \
 -I. -Iexternal/ -Lexternal/
#CFLAGS = -ggdb -Wall -Wno-pointer-sign -std=c90 -ggdb -D_GNU_SOURCE \
# -Iexternal/ -Lexternal/

LIBS = -pthread

SOURCES= external/whirlpool.c external/mt19937ar.c external/sha2.c \
 peng_ref.c peng.c peng_misc.c lpeng.c

TARGETS= $(addsuffix .o, $(basename $(SOURCES)))

EXTRA= TODO README LICENSE CHANGELOG

##############################################################################

all: peng countbits

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFS) $< -o $@ $(LIBS)

peng: $(TARGETS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFS) -o peng $(TARGETS) $(LIBS)

countbits: countbits.c
	$(CC) $(CFLAGS) -o countbits countbits.c

clean:
	rm -f peng *.o core countbits
	rm -f testfile* *~
	rm -f external/*.o

test:
	./dobench.sh

##############################################################################

ci: clean
	git add *.c *.h Makefile* $(EXTRA) scripts/*.?? external/*
	git ci

bump: clean
	./scripts/updver.py
	git add *.c *.h Makefile* $(EXTRA) scripts/*.?? external/*
	git ci

push: bump
	git push savannah

keywords:
	git-keyw-filter *.cc *.hh Makefile*

##############################################################################

tarball: $(SOURCES)
	./scripts/000_genbsd_shebang.sh
	./scripts/mktarball.sh peng4-$(shell ./scripts/getver.sh).tar.bz2 Makefile* $(EXTRA) *.c *.h scripts/* external/* docs/*

