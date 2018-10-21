CC := clang
OUTDIR = bin
OBJ = common.o net.o image.o crypto.o flag.o img_db.o img_cache.o
CFLAGS = -std=c11 -fno-PIC -m32 -O2 -Wall -Wextra -ftree-vectorize -msse2 -msse3
LIB = -lsqlite3 -lpthread
AC_PATH = abob_cloud_service
LD_PATH = loader

service: $(patsubst %,$(OUTDIR)/%,$(OBJ)) $(OUTDIR)/sha256.o $(OUTDIR)/upng.o
	$(CC) $(AC_PATH)/main.c $^ $(CFLAGS) $(LIB) -o$(OUTDIR)/abob_cloud

$(OUTDIR)/%.o: $(AC_PATH)/%.c $(AC_PATH)/%.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)/sha256.o: $(AC_PATH)/SHA256/sha256.c $(AC_PATH)/SHA256/sha256.h
	$(CC) -c -o$@ $< $(CFLAGS)

$(OUTDIR)/upng.o: $(AC_PATH)/upng/upng.c $(AC_PATH)/upng/upng.h
	$(CC) -c -o$@ $< $(CFLAGS)

loader_bin:
	$(CC) $(LD_PATH)/loader.c $(CFLAGS) -o $(OUTDIR)/loader

preloader_bin: service loader_bin
	objcopy --add-section .rodat=$(OUTDIR)/abob_cloud --set-section-flags .rodat=data,readonly $(OUTDIR)/loader $(OUTDIR)/preloader

get_section: preloader_bin
	objdump -x $(OUTDIR)/preloader | grep '.rodat ' | awk '{print $$3, $$6}' | sed -E 's/\s0+/ /' | sed -E 's/^0+//' | awk '{print "0x" $$1, "0x" $$2}'

packed_loader: preloader_bin
	$(OUTDIR)/preloader 421bee0d7d772f5489192ae430037ac0
	objcopy --add-section .rodat=/tmp/abob_cloud --set-section-flags .rodat=data,readonly $(OUTDIR)/loader $(OUTDIR)/real_loader

check_loader: packed_loader
	$(OUTDIR)/real_loader 421bee0d7d772f5489192ae430037ac0
	file /tmp/abob_cloud | grep ELF

clean:
	rm -f /tmp/abob_cloud
	rm -f $(OUTDIR)/*
