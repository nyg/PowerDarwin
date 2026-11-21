CFLAGS = -arch ppc -Os -g -Wall -Werror

install:
	mkdir -p $(SYMROOT) $(DSTROOT)/usr/bin
	gcc-4.0 $(CFLAGS) -o $(SYMROOT)/openplay $(SRCROOT)/openplay.c -framework IOKit -framework CoreFoundation
	install -m 755 $(SYMROOT)/openplay $(DSTROOT)/usr/bin/openplay
	strip $(DSTROOT)/usr/bin/openplay

installhdrs:
