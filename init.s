		.file	"init.s"

		.set	PDT,	0x100000	# put the page table directory at 1 MB

		.section	.init
start:	.globl start
		# first of all, make bloody certain that the A20 line is enabled
		call	waitkb
		movb	$0xd1,%al			# write value to output port command
		outb	%al,$0x64
		call	waitkb
		movb	$0xdf,%al			# data to write (A20 is bit 1)
		outb	%al,$0x60
		call	waitkb
		movb	$0xff,%al
		outb	%al,$0x64
		call	waitkb
		# determine the number of pages required by the kernel
		movl	$end,%ecx
		subl	$stext,%ecx
		addl	$0xfff,%ecx
		shrl	$12,%ecx			# convert to pages
		movl	%ecx,kernelSize		# save for later use
		# get the total amout of physical memory for setting up the page tables
		# this leaves the address hole from 0xa0000 to 0xfffff addressable as
		# there are usualy memory mapped devices there (eg video) and the bios
#		call	get_base_memory
#		movl	%eax,%edx
#		movl	$160,%edx
		call	get_extended_memory
		movl	%eax,%ecx
		addl	$256,%ecx			# total available memory in pages (includes base ram
									# and the address hole)
		subl	kernelSize,%ecx		# do not set up the pages taken by the kernel
									# also gives the first physical page of the kernel
		movl	%ecx,kernelBase		# save the amount
		movl	kernelSize,%ecx		# number of pages for the kernel
		addl	$1023,%ecx			# round up
		shrl	$10,%ecx			# number of page tables required for the kernel
		movl	%ecx,numPageTables	# save the number of page tables to fill in
		addl	$2,%ecx				# one page for base memory and one for the PTD
		movl	kernelBase,%ebp		# physical base of the kernel
		subl	%ecx,%ebp
		shll	$12,%ebp			# %ebp now points to the physical page for the PTD
									# (%ds==%ss)
		cmpl	$0x100000,%ebp		# have we run out of memory already?
		jb		out_of_memory
		# start by filling in the page table directory
		movl	%ebp,%edi
		movl	%ebp,%eax
		addl	$0x1000,%eax		# the first page table just above the PTD
		orl		$0b111,%eax			# user,writable,present
		movl	$1,%ecx				# base memory only (covers first 4MB)
		call	setupPageTable
		# initialize the page table for base memory (only the first MB will be
		# accessable) (edi already points to the right location)
		xorl	%eax,%eax			# start at page 0
		orl		$0b111,%eax			# user,writable,present
		movl	$256,%ecx			# 1MB of memory in pages
		call	setupPageTable
		# now initialize the kernel address space (edi already points to the right
		# location)
		# find out which page directory table entry to set
		movl	$stext,%ecx
		shrl	$22,%ecx			# convert to index into PDE
		pushl	%edi
		movl	%edi,%eax
		leal	(%ebp,%ecx,4),%edi
		orl		$0b011,%eax			# supervisor,writable,present
		movl	numPageTables,%ecx
		call	setPageTableEntries
		popl	%edi
		# determine where in physical memory to place the kernel
		movl	kernelBase,%eax		# total physical pages - pages needed by kernel
		shll	$12,%eax			# convert to address
1:		orl		$0b011,%eax			# supervisor,writable,present
		movl	$1024,%ecx
		cmpl	kernelSize,%ecx		# number of pages required by kernel
		jb		2f
		movl	kernelSize,%ecx
2:		subl	%ecx,kernelSize
		jz		3f
		call	setupPageTable
		jmp		1b
3:		call	setupPageTable
		# set up the very last page table in the page table directory to point to the
		# page table directory!!  This might sound strange, but it makes accessing the
		# page tables (and directory) easier when in paged mode (thanks to the 386BSD
		# project).  The price is 4Mb of memory is no longer usable by the system for
		# other purposes, but if 4Gb was being used, 4Mb would be needed to store the
		# page tables anyway.
		#
		# leal	interresting_logical_address,%eax
		# shrl	$12,%eax						# get INDEX of page table entry
		# movl	0xffc00000(,%eax,4),%eax		# get the page table entry into %eax
		#
		# The last 1024 entries will actually refer to the page table directory entries
		# and thus the physical addresses and attributes of the page tables.
		movl	%ebp,%eax			# physical address of PTD
		orl		$0b011,%eax			# supervisor,writable,present
		movl	%eax,0xffc(%ebp)	# last entry in the page directory table
		# initialize cr3 to point to the page table directory
		movl	%ebp,%cr3
		movl	%ebp,%eax
		# wave our hands about and start up paging
		movl	%cr0,%eax
		orl		$0x80000000,%eax
		movl	%eax,%cr0
		jmp		0					# flush the instruction pipeline
		# now move the kernel data loaded from the disk to where it belongs
		movl	$end,%ecx
		subl	$stext,%ecx
		addl	$3,%ecx
		shrl	$2,%ecx
		movl	$stext,%edi
		movl	$einit,%esi
		cld
		movl	(%edi),%eax
		rep
		movsl
		# the kernel can now be run
		jmp		startup

		.align	2
waitkb:
		xorl	%ecx,%ecx
1:		jmp		0
		jcxz	0
		inb		$0x64,%al
		testb	$2,%al
		loopnz	1b
		ret

		.align	2
setupPageTable:
		movl	$1024,%edx
		subl	%ecx,%edx
		# fill in the specified number of page table entries with contiguous addresses
		jecxz	2f
1:		stosl
		addl	$0x1000,%eax
		loop	1b
2:		movl	%edx,%ecx
		# fill in any page table entries with null pages
		jecxz	3f
		xorl	%eax,%eax			# inaccessable page
		rep
		stosl
3:		ret

setPageTableEntries:
		jecxz	2f
		stosl
1:		addl	$0x1000,%eax
		loop	1b
2:		ret

		.align	2
read_cmos:
		# make sure the nmi mask (bit 7) is set to disable the bloody things
		orb		$0x80,%al
		# set the cmos register adress
		outb	%al,$0x70
		jmp		0
		inb		$0x71,%al
		ret

		.align	2
get_base_memory:
		xorl	%ecx,%ecx
		movb	$0x15,%al			# base memory low byte
		call	read_cmos
		movb	%al,%cl
		movb	$0x16,%al			# base memory high byte
		call	read_cmos
		movb	%al,%ch
		shrl	$2,%ecx				# cmos base memory is in kb, convert to pages
		movl	%ecx,%eax
		ret

		.align	2
get_extended_memory:
		xorl	%ecx,%ecx
		movb	$0x17,%al			# extended memory low byte
		call	read_cmos
		movb	%al,%cl
		movb	$0x18,%al			# extended memory high byte
		call	read_cmos
		movb	%al,%ch
		shrl	$2,%ecx				# cmos extended memory is in kb, convert to pages
		movl	%ecx,%eax
		ret

		.align	2
out_of_memory:
		leal	dieing_a_sad_and_horrible_death,%esi
		call	printString
		cli
		hlt

		.align	2
printEAX:
		pushl	%eax
		shrl	$16,%eax
		call	printAX
		movl	(%esp),%eax
		call	printAX
		movl	cursor,%eax
		movl	$0x07200720,(%eax)
		addl	$4,cursor
		popl	%eax
		ret

		.align	2
printECX:
		pushl	%eax
		movl	%ecx,%eax
		call	printEAX
		popl	%eax
		ret

		.align	2
printEDI:
		pushl	%eax
		movl	%edi,%eax
		call	printEAX
		popl	%eax
		ret

		.align	2
printAX:
		pushl	%eax
		movb	%ah,%al
		call	printAL
		popl	%eax
		.align	2
printAL:
		pushl	%eax
		shrb	$4,%al
		call	printNIB
		popl	%eax
		.align	2
printNIB:
		andb	$0xf,%al
		aam
		addb	%ah,%al
		.byte	0xd5,0x10			# aad 0x10
		addb	$'0',%al
		.align	2
printChar:
		movb	$0x07,%ah
		pushl	%ebx
		movl	cursor,%ebx
		movw	%ax,(%ebx)
		addl	$2,%ebx
		movl	%ebx,cursor
		popl	%ebx
		ret

		.align	2
printString:
		pushl	%esi
		lodsb
		orb		%al,%al
		jz		1f
		call	printChar
		jmp		printString
1:		popl	%esi
		ret

		.align	2
cursor:
		.long	0xb80a0
kernelSize:
		.long	0
kernelBase:
		.long	0
numPageTables:
		.long	0
dieing_a_sad_and_horrible_death:
		.asciz	"Argh... insufficient extended memory! Halting system."
