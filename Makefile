CC ?= clang
OUTDIR = bin
OBJ = common.o net.o image.o crypto.o flag.o img_db.o
CFLAGS = -std=c11 -fno-PIC -m32 -O2 -Wall -Wextra
LIB = -lsqlite3 -lpthread
AC_PATH = abob_cloud_service

all: $(patsubst %,$(OUTDIR)/%,$(OBJ)) $(OUTDIR)/sha256.o $(OUTDIR)/upng.o
	$(CC) $(AC_PATH)/main.c $^ $(CFLAGS) $(LIB) -o$(OUTDIR)/abob_cloud

$(OUTDIR)/%.o: $(AC_PATH)/%.c $(AC_PATH)/%.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)/sha256.o: $(AC_PATH)/SHA256/sha256.c $(AC_PATH)/SHA256/sha256.h
	$(CC) -c -o$@ $< $(CFLAGS)

$(OUTDIR)/upng.o: $(AC_PATH)/upng/upng.c $(AC_PATH)/upng/upng.h
	$(CC) -c -o$@ $< $(CFLAGS)

clean:
	rm -f bin/*
