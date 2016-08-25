CC = m68k-palmos-gcc
CFLAGS = -O2 -palmos2

all : juke.prc

juke.prc : code0001.juke.grc tfrm03e8.bin
	build-prc juke.prc "Juke" TxHx *.grc *.bin 
	ls -l *.prc

tfrm03e8.bin: juke.rcp juke.h juke.bmp
	pilrc juke.rcp 

code0001.juke.grc: juke.c juke.h
	$(CC) $(CFLAGS) -c juke.c -o juke.o 
	$(CC) $(CFLAGS) juke.o -o juke
	m68k-palmos-obj-res juke

clean:
	-rm -f *.[oa] juke *.bin *.stamp *.grc
