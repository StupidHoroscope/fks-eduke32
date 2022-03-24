#!/bin/sh

# Detect whether libvorbis libs are already there then copy them if not
if [ ! -f /usr/lib/libvorbis.so.0 ]; then
	rw
	cp -f libvorbis.so.0 /usr/lib
	cp -f libvorbisfile.so.3 /usr/lib
	./eduke32
	pid record $!
	wait $!
	pid erase
	rm -f /usr/lib/libvorbis.so.0
	rm -f /usr/lib/libvorbisfile.so.3
	ro
else
	./eduke32 &
#	./eduke32 -instantplay &> /mnt/FunKey/.eduke32/stdout.log	
	pid record $!
	wait $!
	pid erase
fi
