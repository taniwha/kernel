		.type	"bin"
reloc	=		0x0e00				; base address for the code
		.start
		; first thing, set up the stack to just below where we're going to put
		; the code (code goes to 0xe00 linear).  Set SS to 0 and sp to 0xe00
		; so that when we go to pmode with our flat address space, all we have
		; to do to esp is mask out the upper 16 bits, leaving the lower 16
		; bits however they get set by the code (this way info can be passed
		; to the coff file via the stack as well as registers, but this is not
		; done in this version, just the registers).
		xor		ax,ax				; setup stack pointer for 0:0e00
		mov		ss,ax
		mov		sp,reloc
		; bios puts us at 0x7c00 but we want to be at 0x0e00 (coff image gets
		; loaded at 1000).  However, djasm writes out the first 0x0e00 byts if
		; we try to set the origin at 0x0e00, so we instead, use intel's
		; segments for what they were meant for... auto relocation! (see, they
		; *DO* have a purpose, all you segment bashers. ok ok, segments are a
		; pain in the but for usefull appications (what about the good old
		; apple ][ with 48k ram (load runner was pretty cool @~32-48k)). Well
		; anyway, we set the code segment (and the data segemts) to 0xe0
		; (0x0e00 linear) so that our 0 based offsets are correct. Only
		; problem is that we have to remember to add 0x0e00 to our offsets to
		; get linear addresses (eg. for the lgt struct)
		mov		di,ax				; where we want to go (e0:0)
		mov		al,reloc>>4			; segment pointing to e0:0 (ah alreay 0)
		mov		es,ax				; point es and ds to where we want to go
		mov		ds,ax
		mov		cx,0x0100			; 256 words to move
		mov		si,0x7c00 - reloc	; bios loaded us here (relative to e0:0)
		rep							; heave...
		movsw						; ho...
		jmpf	(reloc>>4):continue	; set cs to 0xe0 (and skip error handler)
		; this code is placed here to avoid having to use long jumps (saves a
		; byte per jump and they won't work on <386 anyway)
not386:	; hey, you can't run 386 code on a 286 or less
		mov		cx,message2_length
		jmp		print_msg
error:	; boot error (MOST probable cause, forgot to copy the coff file to the
		; correct location on the disk
		mov		cx,message1_length
print_msg:
		; print the message, wait for a key, then reboot the computer via int
		; 19
		push	cx			; save cx (string len)
		mov		bx,0x0007	; page 0, black background, light grey forground
		mov		ah,3		; get cursor data
		int		0x10		; bios video services
		pop		cx			; restore string len (not interested in cursor
							; shape)
		mov		ax,0x1301	; write string (cursor moves, no attributes)
		push	cs
		pop		es
		mov		bp,message	; we always print the same message (just not all
							; of it sometimes)
		int		0x10		; bios video services
		mov		ah,0		; get key
		int		0x16		; bios keyboard services
		int		0x19		; lets try that one again (reboot from floppy)
forever:					; hey, how did we get here?????
		sti					; allow interrupts (dunno why, but seems like a
							; good idea)
		hlt					; zzzz...
		jmp		forever		; snort...
continue:
		; first of all, test to see if we're on a 386 or better, no point in
		; wasting time loading code we can't run.  Uses the flags method of
		; cpu detections. 8088/86 *ALWAYS* sets bits 12-15 of the flags
		; register and the 286 *ALWAYS* clears them.
		pushf
		pop		ax
		and		ah,0x0f
		mov		bh,ah
		push	ax
		popf
		pushf
		pop		ax
		and		ah,0xf0
		cmp		ah,0xf0		; if all bits 12-15 set, 
		je		not386		; oops, 8086/88
		mov		ah,bh
		or		ah,0xf0
		push	ax
		popf
		pushf
		pop		ax
		and		ah,0xf0		; if all bits 12-15 clear,
		jz		not386		; oops, 286
		; ok, it's ok to load the coff file. However, only the first sector is
		; read in before the magic numbers are checked. Again, no point in
		; loading code we can't run.
		mov		bx,(reloc>>4)
		mov		es,bx
		mov		bx,coff_image ; this will be 0x1000 linear. djgpp coff files
							; ALL use this as there base virtual address, but
							; omit the first 4k of data as the first page is
							; used to catch null pointer accesses (when the
							; dpmi provider provides the nessecary services,
							; which windows does not but cwsdpmi does). This
							; means that we can just load the file at 0x1000
							; linear (using flat segments) and be done with
							; it, all internal offsets will be correct
		mov		ax,[load_sector] ; load ax with low word of start sector
		mov		dx,[load_sector+2] ; load dx with high word of start sector
		mov		cx,[load_count] ; number of sectors to load
		jcxz	error		; oop's! misconfigured boot sector
		call	read_sector	; read in the first sector
		jc		error		; buzz, bad disk
		cmpw	[coff_magic],0x014c	; djgpp coff magic number
		jne		error		; buzz
		cmpw	[coff_aout_magic],0413 ; djgpp a.out magic number (in opthdr)
							; NOTE ^ is OCTAL!!
		jne		error		; buzz
		dec		cx			; whew, made it! One less sector, please
		jcxz	enter_pmode	; what! only one sector (wee os is it?:)
							; more likely just the test code
@1b:
		add		ax,1		; point to the next sector
		adc		dx,0		; 32 bit addition using 16 bit regs :(-
		mov		si,es		; bump up es by 512 bytes, 32(0x20) paragraphs
		add		si,0x20
		mov		es,si
		call	read_sector	; read the next sector
		jcl		error		; arrrg...
		loop	@1b			; round and round the mulberry bush....
enter_pmode:
		; first mask out both hardware interrupts and the NMI interrupt (I
		; hope) if an interrupt occurs while were going from real mode to
		; protected mode, the cpu will shut down (or lock up) because no
		; matter how you do it, there will *ALWAYS* be a time where the
		; interrupt vector table is invalid for the mode were in.
		cli					; shhh... (hardward interrupts)
		in		al,0x70		; and you too! (NMI interrupt)
		or		al,0x80		; this is SUPPOSED to mask out the NMI. I have no
		out		0x70,al		; personal experience with this, but this is what
							; Ralph Brown's interrupt list (ports.lst
							; actually) says. Sooo, trust the experts...
		; load up some relevant data into registers to pass into pmode. This
		; probably isn't nessecary (the coff can just look at locations 0x9f0
		; to 0x9fe and extract the data itself) so I might kill this soon.
		mov		ebx,[load_sector]
		mov		ecx,[sectors_per_track]
		mov		edx,[boot_drive]
		; push the code selector and coff entry point onto the stack. Saves
		; having to use self modifying code which I like to avoid as it's
		; dificult to debug and understand, and unnessacary in this instance
		pushd	0x08		; code descriptor
		pushd	[coff_aout_start] ; location to start execution in the 32 bit
							; segment
		lidt	[idtp]		; kill the interrupt descriptor table
		lgdt	[gdt]		; load up the global descriptor table
		; usual boring 'enter protected mode' incantation and hand waving
		mov		eax,cr0
		or		al,1
		mov		cr0,eax
		; now in protected mode
		jmp		@1f			; flush the prefetch buffer
@1f:	; weeere heeeere...
		; load up the data type segment registers with the data segment (0x10)
		; and then do a pseudo intersegment jump to the coff file using a far
		; return (the stack has already been set up above by the two pushd
		; instructions).
		mov		ax,0x10		; data descriptor
		mov		ss,ax		; stack uses the data segment
		and		esp,0xffff	; make sure high bits if esp are 0 (low bits
							; already set)
		mov		ds,ax		; make all segment registers hold valid pmode
		mov		es,ax		; selectors (the data segment)
		mov		fs,ax
		mov		gs,ax
		; been nice knowing you. please be nice to the os.
		retfd				; jump to 8:*coff_entry
		
read_sector:
		; read in a single sector
		; es:bx = transfer buffer
		; dx:ax = sector to read
		;
		push	ax			; save the regs
		push	dx
		push	cx			; we use this but it's used in the read loop
		push	si			; hey, save all used regs!!
		divw	[sectors_per_track]
		mov		cl,dl		; sector (remainder of div) goes into cl
		inc		cl			; sectors start at 1 (dumb idea that)
		xor		dx,dx		; should never be more that 64k tracks
		divw	[tracks_per_cylinder]
		mov		ch,al		; bits 0-7 of track into ch
		shl		ah,6		; move bits 8,9 of cylinder into bits 6,7 of dh
;		ror		ah,1		; bloody braindead bios with max 1024 cyls
;		and		ah,0xc0		; mask out unused bits
		or		cl,ah		; bits 8,9 of cylinder in bits 6,7 of cl (bbdb!!)
		mov		dh,dl		; track number into dh
		mov		dl,[boot_drive]
		mov		si,3		; three reties
rs_loop:
		mov		ax,0x0201	; read one sector
		int		0x13		; bios disk services
		jnc		rs_exit		; carry set=error, not set=hunky dory
		dec		si			; does not affect carry flag
		je		rs_exit		; oop's sumpin wong wit du disk
		mov		ah,0		; reset the disk system before we try again
		int		0x13		; bds
		jmp		rs_loop		; play it again, sam
rs_exit:
		pop		si			; restore regs
		pop		cx
		pop		dx
		pop		ax
		ret
message:; the first part of the message is reused (well, you can't boot from
		; this disk if the machine isn't a 386 or better, so it's approriate)
		.db		"Can't boot from this disk\r\n"
		message1_length = . - message
		.db		"Not 386+, system halted (not)..."
		message2_length = . - message
		.org	0x1c0,0xff
idtp:	; no real idt for now. If an intterupt occurs, the cpu will shutdown
		; and promptly reset (via motherboard), but this shouldn't happen as
		; both hardware interrupts and the NMI (I hope) have been disabled.
		.dw		0
		.dd		0
		.align	8,0
gdt:
		; nul descriptor, ix86 allows (but ignores) non zero descriptor data
		; does double duty as gdt pointer (cudos to Rober Colins' page at
		; www.x86.org)
		.dw		3*8-1		; limit = 3 selectors
		.dd		gdt+reloc	; hey, that's me! :)
		.dw		0			; not used, pads out to 8 bytes (2+4+2)
		; 32 bit flat code descriptor
		.dw		0xffff	; limit (0-15) = 0xfffff
		.dw		0x0000	; base (0-15)
		.db		0x00	; base (16-23)
		.db		0x9a	; not accessed, readable code, ring 0, present
		.db		0xcf	; limit (16-19), use 32, 4k gran
		.db		0x00	; base (24-31)
		; 32 bit flat data descriptor
		.dw		0xffff	; limit (0-15) = 0xfffff
		.dw		0x0000	; base (0-15)
		.db		0x00	; base (16-23)
		.db		0x92	; not accessed, writeable data(up), ring 0, present
		.db		0xcf	; limit (16-19), use 32, 4k gran
		.db		0x00	; base (24-31)
		; volume specific information fills the last 30 bytes of the boot
		; sector
		.org	0x1e0,0xff
root_sector:
		.dd		0		; starting sector of root directory
root_size:
		.dd		0		; number of sectors in root directory
secmap_sector:
		.dd		0		; starting sector of sector allocation bitmap
secmap_size:
		.dd		0		; number of sectors in sector allocation bitmap (needed?)
		.org	0x1f0,0xff
load_sector:			; starting sector of coff image to load
		.dd		1
load_count:				; how big it is (allows 32Mb, but I'm not M$, so 'nuf)
		.dw		1
sectors_per_track:		; number of sectors on each track (18 for 1.44 floppy)
		.dw		18
tracks_per_cylinder:	; number of tracks per cylinder (2 for floppies)
		.dw		2
boot_drive:
		.db		0		; the disk we booted from
		; the next three bytes are available for future definition, if you
		; come up with some use for them, let me know, huh.
		.db		0		; reserved
		.dw		0
		; obligatory bios doodad (int 19 won't recognize us without this)
		.org	0x1fe	; this will catch any overflow (code getting too big)
		.dw		0xaa55	; bios validation flag
		.align	512,0	; error checking, will cause the bin file to go to
						; 1024 bytes if there is any overflow above.
		.bss			; no bytes will be generated from this point on
coff_image:				; this is where the coff file will be loaded and will
						; always have the address of 0x1000 if there is no
						; overflow above
		; the following are the offsets of the magic numbers we want to check
		; and the entry point of the coff image.
coff_magic = coff_image	; coff magic number
coff_opthdr = coff_image + 16 ; opthdr size (not used)
coff_aout = coff_image + 20	; opthdr offset
coff_aout_magic = coff_aout ; a.out magic number
coff_aout_start = coff_aout + 16 ; entry point

free_bytes = idtp - (message + message2_length)
