KEXT = $(DSTROOT)/System/Library/Extensions/OpenFWOHCI.kext
KERNEL_HEADERS = /System/Library/Frameworks/Kernel.framework
CFLAGS = -arch ppc -fno-common -nostdinc -Os -g -Wall -Werror \
	-I$(KERNEL_HEADERS)/PrivateHeaders -I$(KERNEL_HEADERS)/Headers \
	-fno-builtin -finline -fno-keep-inline-functions -force_cpusubtype_ALL -msoft-float -mlong-branch -static \
	-DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -fcheck-new -fapple-kext

install:
	mkdir -p $(OBJROOT) $(SYMROOT) $(KEXT)/Contents/MacOS
	g++-4.0 -c $(CXXFLAGS) -o $(OBJROOT)/OpenFWOHCI.o $(SRCROOT)/OpenFWOHCI.cpp
	g++-4.0 -c $(CXXFLAGS) -o $(OBJROOT)/Async.o $(SRCROOT)/Async.cpp
	gcc-4.0 -c $(CFLAGS) -o $(OBJROOT)/OpenFWOHCI_info.o $(SRCROOT)/OpenFWOHCI_info.c
	g++-4.0 -arch ppc -static -nostdlib -r -o $(SYMROOT)/OpenFWOHCI $(OBJROOT)/OpenFWOHCI.o $(OBJROOT)/Async.o $(OBJROOT)/OpenFWOHCI_info.o -lkmodc++ -lkmod
	install -m 644 $(SRCROOT)/Info.plist $(KEXT)/Contents/Info.plist
	install -m 755 $(SYMROOT)/OpenFWOHCI $(KEXT)/Contents/MacOS/OpenFWOHCI
	strip -S $(KEXT)/Contents/MacOS/OpenFWOHCI

installhdrs:
