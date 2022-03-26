#!/bin/sh

# Detect whether libvorbis libs are already there then copy them if not
# (This is due to some OS versions missing the necessary files)
if [ ! -f /usr/lib/libvorbis.so.0 ]; then
	rw
	cp -f libvorbis.so.0 /usr/lib
	cp -f libvorbisfile.so.3 /usr/lib
	./eduke32 &
	pid record $!
	wait $!
	pid erase
	ro
else
	./eduke32 &
	pid record $!
	wait $!
	pid erase
fi
