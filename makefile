CPPFLAGS=
CFLAGS=-g -O2 -Wall -Werror
CXXFLAGS=$(CFLAGS)

#%.o: %.s
#	as -o $@ $<

%.exe: %.c
	gcc $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^
#	djp $@

%.exe: %.cc
	gcc $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^
#	djp $@

%.o: %.S
	gcc $(CPPFLAGS) -c $<

%.d: %.c
	$(MAKEDEPS)

%.d: %.cc
	$(MAKEDEPS)

#%.d: %.s
#	echo $*.o $@: $<> $@

%.d: %.S
	$(MAKEDEPS)

MAKEDEPS=$(CC) -MM $(CPPFLAGS) $< | sed -e 's/$*\.o:*/$*\.o $@:/g' > $@

BOOT_sources=\
 init.s		\
 startup.s	\
 __main.c	\
			\
 inter.S	\
 traps.c	\
			\
 xmm.c		\
 physmem.c	\
 ksbrk.c	\
 kmalloc.c	\
			\
 screen.c	\
			\
 fdc.c		\
 ide.c		\
 lfsys.c	\
 bc_fsys.c	\
 			\
 selectors.c\
 s386.s		\
 dma.s		\
 irq.S		\
 keybdio.s	\
 cmos.s		\
 textmode.s	\
 textfont.s	\
 vm86.S		\
 rmcb.c		\
 serio.s	\
 main.c		\
 paging.S	\
 kprintf.c	\
 kgets.c	\
			\
 dos.c		\
 dos_memory.c\
 dos_disk.c

BOOT_objects=$(addsuffix .o,$(basename $(BOOT_sources)))

BOOT_dependencies=$(addsuffix .d,$(basename $(BOOT_sources)))

all: bootstrap.bin boot copyboot.exe

cb: all
	copyboot

copyboot.exe: copyboot.cc -liostream

bootstrap.bin: bootstrap.asm
	djasm bootstrap.asm bootstrap.bin bootstrap.map

boot: boot.ld $(BOOT_objects)
#boot: boot.ld test.o
	ld -Map $@.map -o $@ -T$^

clean:
	rm -f boot *.o *.exe *.bin *.map *.d *.ah err

vm86.d: qsir.ah
keybdio.d: keybdio.s
	echo keybdio.d keybdio.o: keybdio.s>keybdio.d
dma.d: dma.s
	echo dma.d dma.o: dma.s>dma.d
keybdio.o: keybdio.s
	as -o $@ $^
dma.o: dma.s
	as -o $@ $^

-include $(BOOT_dependencies)

qsir.ah: qsir.asm
	djasm qsir.asm qsir.ah
serio.d: serio.s
	echo serio.o: serio.s>serio.d
	echo "	as serio.s -o serio.o">>serio.d
