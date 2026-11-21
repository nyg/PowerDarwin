KEXT = $(DSTROOT)/System/Library/Extensions/OpenOnyx.kext
KERNEL_HEADERS = /System/Library/Frameworks/Kernel.framework
AOA_HEADERS = /usr/local/include/AppleOnboardAudio
CFLAGS = -arch ppc -fno-common -nostdinc -Os -g -Wall \
	-I$(AOA_HEADERS) -I$(KERNEL_HEADERS)/PrivateHeaders -I$(KERNEL_HEADERS)/Headers \
	-fno-builtin -finline -fno-keep-inline-functions -force_cpusubtype_ALL -msoft-float -mlong-branch -static \
	-DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -fcheck-new -fapple-kext

install:
	mkdir -p $(OBJROOT) $(SYMROOT) $(KEXT)/Contents/MacOS
	g++-4.0 -c $(CXXFLAGS) -o $(OBJROOT)/OpenOnyx.o $(SRCROOT)/OpenOnyx.cpp
	gcc-4.0 -c $(CFLAGS) -o $(OBJROOT)/OpenOnyx_info.o $(SRCROOT)/OpenOnyx_info.c
	g++-4.0 -arch ppc -static -nostdlib -r -o $(SYMROOT)/OpenOnyx $(OBJROOT)/OpenOnyx.o $(OBJROOT)/OpenOnyx_info.o -lkmodc++ -lkmod
	install -m 644 $(SRCROOT)/Info.plist $(KEXT)/Contents/Info.plist
	install -m 755 $(SYMROOT)/OpenOnyx $(KEXT)/Contents/MacOS/OpenOnyx
	strip -S $(KEXT)/Contents/MacOS/OpenOnyx

installhdrs:
