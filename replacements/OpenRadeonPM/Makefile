KEXT = $(DSTROOT)/System/Library/Extensions/OpenRadeonPM.kext
KERNEL_HEADERS = /System/Library/Frameworks/Kernel.framework
CFLAGS = -arch ppc -fno-common -nostdinc -Os -g -Wall -Werror \
	-I$(KERNEL_HEADERS)/PrivateHeaders -I$(KERNEL_HEADERS)/Headers \
	-fno-builtin -finline -fno-keep-inline-functions -force_cpusubtype_ALL -msoft-float -mlong-branch -static \
	-DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -fcheck-new -fapple-kext

install:
	mkdir -p $(OBJROOT) $(SYMROOT) $(KEXT)/Contents/MacOS
	g++-4.0 -c $(CXXFLAGS) -o $(OBJROOT)/OpenRadeonPM.o $(SRCROOT)/OpenRadeonPM.cpp
	gcc-4.0 -c $(CFLAGS) -o $(OBJROOT)/OpenRadeonPM_info.o $(SRCROOT)/OpenRadeonPM_info.c
	g++-4.0 -arch ppc -static -nostdlib -r -o $(SYMROOT)/OpenRadeonPM $(OBJROOT)/OpenRadeonPM.o $(OBJROOT)/OpenRadeonPM_info.o -lkmodc++ -lkmod
	install -m 644 $(SRCROOT)/Info.plist $(KEXT)/Contents/Info.plist
	install -m 755 $(SYMROOT)/OpenRadeonPM $(KEXT)/Contents/MacOS/OpenRadeonPM
	strip -S $(KEXT)/Contents/MacOS/OpenRadeonPM

installhdrs:
