CC ?= clang
OUTDIR = bin
OBJ = common.o net.o image.o crypto.o flag.o img_db.o serv.o
CFLAGS = -std=c11 -fno-PIC -m32 -O2 -Wall -Wextra -g
LIB = -lsqlite3 -lpthread

all: $(patsubst %,$(OUTDIR)/%,$(OBJ)) $(OUTDIR)/ecc.o $(OUTDIR)/sha256.o
	$(CC) main.c $^ $(CFLAGS) $(LIB) -o$(OUTDIR)/abob_cloud

$(OUTDIR)/%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)/ecc.o: easy-ecc/ecc.c easy-ecc/ecc.h
	$(CC) -c -o$@ $< $(CFLAGS) -DECC_CURVE=secp256r1

$(OUTDIR)/sha256.o: SHA256/sha256.c SHA256/sha256.h
	$(CC) -c -o$@ $< $(CFLAGS)

clean:
	rm -f bin/*
