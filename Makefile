CFLAGS=-Wall -Wextra -pedantic -std=c99 -D_BSD_SOURCE -march=i386 -m32 -ffunction-sections -fdata-sections -D__BSD_VISIBLE -D_FILE_OFFSET_BITS=64

LDFLAGS=$(CFLAGS) -march=i386 -m32 -ffunction-sections -fdata-sections 
DUMPFLAGS=-dead-strip -dead_strip_dylibs -static 

.PHONY: clean

all: dvenue_unpack dvenue_dump_images_pc dvenue_dump_images_s #dvenue_pack 

clean:
	rm -f *.o dvenue_unpack dvenue_dump_images dvenue_dump_images_s dvenue_dump_images_pc dvenue_pack 

dvenue_pack: dvenue_pack.o
	$(CC) -o $@ $< $(CFLAGS)

dvenue_unpack: dvenue_unpack.o
	$(CC) -o $@ $< $(CFLAGS) 

dvenue_dump_images_pc.o: dvenue_dump_images.c
	$(CC) -c -o $@ $< $(CFLAGS) -DPC

dvenue_dump_images_s: dvenue_dump_images.o
	$(CC) -o $@ $< $(CFLAGS) $(DUMPFLAGS)
	strip $@

dvenue_dump_images: dvenue_dump_images.o
	$(CC) -o $@ $< $(CFLAGS) 

dvenue_dump_images_pc: dvenue_dump_images_pc.o
	$(CC) -o $@ $< $(CFLAGS) 
