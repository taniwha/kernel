		.file	"dma.s"

		.set	FOR_DJGPP,	0		## set to 1 if building for djgpp, otherwise
									## assumes building for the kernel
.if FOR_DJGPP
		.section .ltext
.else
		.text
.endif
# the following arrays allow for easy conversion from DMA channel to IO port

#dcom: DMA command register (write only)
#	7:	DACK# active level (1=active high, 0=active low)
#	6:	DREQ  active level (1=active low, 0=active high)
#	5:	0 (intel)
#	4:	Group arbitration priority (1=rotating, 0=fixed)
#	3:	0 (intel)
#	2:	Group enable (what's that??)
#	1,0:0 (intel)
dcom:	.word 0x08, 0x08, 0x08, 0x08, 0xd0, 0xd0, 0xd0, 0xd0
#dcm: DMA channel Mode Register (write only)
#	7,6:DMA transfer mode
#		00	Demand mode
#		01	Single mode
#		10	Block mode
#		11	Cascade mode
#	5:	Address direction (1=down, 0=up)
#	4:	Autoinitialize (1=enable, 0=disable)
#	3,2:DMA transfer type
#		00	Verify transfer
#		01	Write transfer (io to mem)
#		10	Read transfer  (mem to io)
#		11	invalid
#	1,0:DMA channel select
#		00	0 (4)
#		01	1 (5)
#		10	2 (6)
#		11	3 (7)
dcm:	.word 0x0b, 0x0b, 0x0b, 0x0b, 0xd6, 0xd6, 0xd6, 0xd6
#dr: DMA request register (write only)
#	7-3:0 (intel)
#	2:	Reset (0) or set (1) individual software DMA channel requst
#	1,0:DMA channel select
#		00	0 (4)
#		01	1 (5)
#		10	2 (6)
#		11	3 (7)
dr:		.word 0x09, 0x09, 0x09, 0x09, 0xd2, 0xd2, 0xd2, 0xd2
#smr: Mask register: write single mask bit (write only)
#	7-3:0 (intel)
#	2:	1=disable DREQ, 0=enable DREQ
#	1,0:DMA channel select
#		00	0 (4)
#		01	1 (5)
#		10	2 (6)
#		11	3 (7)
smr:	.word 0x0a, 0x0a, 0x0a, 0x0a, 0xd4, 0xd4, 0xd4, 0xd4
#amr: Mask register: write all mask bits (read/write)
#	7-4:0 (intel)
#	3-0:Channel mask bits
#		3:	3 (7)
#		2:	2 (6)
# 		1:	1 (5)
#		0:	0 (4)
amr:	.word 0x0f, 0x0f, 0x0f, 0x0f, 0xde, 0xde, 0xde, 0xde
#status: DMA status register (read only, shared with dcom)
#	7-4:Channel request status (undefined for channel 4?)
#		7:	3 (7)
#		6:	2 (6)
#		5:	1 (5)
#		0:	0
#	3-0:Channel terminal count (TC) status (1=TC reached)
#		3:	3 (7)
#		2:	2 (6)
#		1:	1 (5)
#		0:	0
status:	.word 0x08, 0x08, 0x08, 0x08, 0xd0, 0xd0, 0xd0, 0xd0
#base: DMA 16 bit page offset (0-15) (read/write)
base:	.word 0x00, 0x02, 0x04, 0x06, 0xc0, 0xc4, 0xc8, 0xcc
#count: DMA transfer count. chan 0-3=bytes (8b), 4-7=words (16b) (read/write)
count:	.word 0x01, 0x03, 0x05, 0x07, 0xc2, 0xc6, 0xca, 0xce
#page: DMA 8 bit page register (16-23). Forms 24 bit address with base
#		(page:base) (read/write)
page:	.word 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a
#cbpr: DMA clear byte pointer register. any value. (write only)
cbpr:	.word 0x0c, 0x0c, 0x0c, 0x0c, 0xd8, 0xd8, 0xd8, 0xd8
#mcr: DMA master clear register (hard reset). any value. (write only)
mcr:	.word 0x0d, 0x0d, 0x0d, 0x0d, 0xda, 0xda, 0xda, 0xda
#cmr: DMA clear mask register. any value. (write only)
cmr:	.word 0x0e, 0x0e, 0x0e, 0x0e, 0xdc, 0xdc, 0xdc, 0xdc

		.set	write_cmd,	0x44
		.set	read_cmd,	0x48
		.set	dma_enable,	0x00
		.set	dma_disable,0x04	

		.align	2
		.globl	_dma_setup
# int dma_setup(int channel, void *buffer, unsigned long len, int dir);
# channel 0-7
# buffer. must be word aligned for channels 4-7
# len=number of *transfers* 1-65536 (0x10000)
# dir!=0->mem to io
# dir==0->io to mem
_dma_setup:
		.set	Channel,	8
		.set	Buffer,		12
		.set	Len,		16
		.set	Dir,		20
		pushl	%ebp
		movl	%esp,%ebp
		pushl	%ebx
		pushl	%esi
		pushl	%edi
		pushfl
		#get and validate the dma channel
		movl	$0,_dma_errno
		movl	Channel(%ebp),%ebx
		cmpl	$7,%ebx
		movl	$0xc,%eax
		ja		1f
		#get the base and page register settings.
		movl	Buffer(%ebp),%edi
		testl	$4,%ebx					# channels 4-7?
		jz		2f
		shrl	$1,%edi					# yes, convert to word address
2:		movl	%edi,%ecx
		shrl	$8,%ecx				#ch:di forms 24 bit address (page(8):offset(16))
		#get controller relative channel number
		movb	%bl,%cl
		andb	$0x03,%cl
		#determine which direction the transfer is going
		xorl	%eax,%eax				#try to prevent z/s extension stalls
		#the next five instructions set al to read_cmd (dir=1) or write_cmd
		#(dir=0) intel sujested optimisation. good for all cpu's as it avoids
		#any jumps. 15 cylcles (386) 8(f)/9(t) (486) for this, but (for 386)
		#would take 17 or 18(19?).  This optimisation is actually best for
		#petiums (saves on branch predictions).
		testl	$-1,Dir(%ebp)
		setne	%al						#al=0x01(dir!=0) or 0x00
		decb	%al						#al=0x00 or 0xff
		andb	$write_cmd-read_cmd,%al	#al=0x00 or write_cmd-read_cmd
		addb	$read_cmd,%al			#al=read_cmd or write_cmd
		addb	%cl,%al					#cl=controller relative channel
		movl	%eax,%esi				#hopefully, no 7 cycle stall
		#set up the dma disable command
		addb	$dma_disable,%cl		#cl=controller relative channel
		#make sure that not too many bytes/words are to be transfered and
		#the transfer does not span a 64k/128k boundary (chans 0-3 and 4-7
		#respectively)
		movl	Len(%ebp),%ebp
		decl	%ebp
		cmpl	$0xffff,%ebp			# more than 64k transfers?
		movl	$2,%eax					# error code 2 (same with below)
		ja		1f
		movl	%ebp,%eax
		addw	%di,%ax					# page overrun?
		movl	$2,%eax					# only the carry flag is wanted
		jc		1f
# ch:di == physical base address
# cl == disable command
# ebx == channel number (index into the above arrays)
# esi == mode command
# ebp == length in dma style (0x0000=1, 0xffff=65536)
# esp still used for the stack (bloody hard to get the stack back otherwise:)
# edx and eax used for port io
		#protect our selves from interrupts
		cli
		#stop any ongoing dma transfer on our channel
		movb	%cl,%al
		andb	$~dma_disable,%cl		#convert to dma_enable
		movw	smr(,%ebx,2),%dx
		outb	%al,%dx
		#clear the byte pointer flip flop
		movw	cbpr(,%ebx,2),%dx
		outb	%al,%dx
		#program the trasfer offset base
		movl	%edi,%eax				#avoid data size prefix
		movw	base(,%ebx,2),%dx
		outb	%al,%dx					#lsb first
		movb	%ah,%al
		outb	%al,%dx					#msb
		#program the page register
		movb	%ch,%al
		movw	page(,%ebx,2),%dx
		outb	%al,%dx
		#program the count register
		movl	%ebp,%eax				#avoid data size prefix
		movw	count(,%ebx,2),%dx
		outb	%al,%dx					#lsb first
		movb	%ah,%al
		outb	%al,%dx					#msb
		#program the transfer mode (only low 8 bits of esi needed)
		movl	%esi,%eax				#avoid data size prefix
		movw	dcm(,%ebx,2),%dx
		outb	%al,%dx
		#enable the transfer. cl has been converted from a disable to an
		#ebable above.
		movb	%cl,%al
		movw	smr(,%ebx,2),%dx
		outb	%al,%dx
		#Success, the dma is now setup for the transfer
		xorl	%eax,%eax
		popfl
		popl	%edi
		popl	%esi
		popl	%ebx
		popl	%ebp
		ret
1:		#Oops, an error occured. eax has been initialized with the dma error
		#code.
		movl	%eax,_dma_errno
		movl	$-1,%eax
		popfl
		popl	%edi
		popl	%esi
		popl	%ebx
		popl	%ebp
		ret

#int dma_reset(int channel);
#int dma_pause(int channel);
#channel 0-7
#these two functions are realy synonyms for each other, as there is little you
#can do to actually reset only one DMA channel (as far as I can tell from
#Intel's docs) without resetting the whole chip, which would not be a good
#idea as that could result in the loss of memory (or are all 7 (channel 4 is
#not usable as it is used to cascade the two dma controllers) channels
#available for use?). Anyway, the DMA has been set up by the bios and there's
#no point messing with the settings as all peripherals expect certain settings
#of DREQ and DACK polarities etc.
		.align	2
_dma_reset:
_dma_pause:
		.set	Channel,	4
		movl	Channel(%esp),%ecx
		cmpl	$7,%ecx
		ja		1f
		movb	%cl,%al
		andb	$3,%al
		orb		$dma_disable,%al
		movw	smr(,%ecx,2),%dx
		outb	%al,%dx
		xorl	%eax,%eax
		ret
1:		movl	$0xc,_dma_errno
		movl	$-1,%eax
		ret

#int dma_resume(int channel);
#channel 0-7
#restart a paused dma transfer.
		.align	2
_dma_resume:
		.set	Channel,	4
		movl	Channel(%esp),%ecx
		cmpl	$7,%ecx
		ja		1f
		movb	%cl,%al
		andb	$3,%al
		movw	smr(,%ecx,2),%dx
		outb	%al,%dx
		xorl	%eax,%eax
		ret
1:		movl	$0xc,_dma_errno
		movl	$-1,%eax
		ret

#unsigned long dma_get_count(int channel);
#channel 0-7
#returns the contents of the count register (0-0xffff)
#will be 0xffff when the transfer is complete (should check the appropriat TC
#bit of the status register)
#should return the current number of transfers remaining (remember 0==1).
#if called without during a transfer without pausing the channel, garbage may
#be returned.
		.align	2
_dma_get_count:
		.set	Channel,	4
		movl	Channel(%esp),%ecx
		cmpl	$7,%ecx
		ja		1f
		pushfl
		cli
		movw	cbpr(,%ecx,2),%dx
		outb	%al,%dx
		xorl	%eax,%eax
		movw	count(,%ecx,2),%dx
		inb		%dx,%al
		movb	%al,%ah
		inb		%dx,%al
		xchgb	%al,%ah
		popf
		ret
1:		movl	$0xc,_dma_errno
		movl	$-1,%eax
		ret

#unsigned long dma_get_base(int channel);
#channel 0-7
#returns the contents of the base register (0-0xffff)
#should return the current page offset
#if called during a transfer without pausing the channel, garbage may be
#returned.
#converts to byte address for channels 4-7 (0-0x1fffe)
		.align	2
_dma_get_base:
		.set	Channel,	4
		movl	Channel(%esp),%ecx
		cmpl	$7,%ecx
		ja		1f
		pushfl
		cli
		movw	cbpr(,%ecx,2),%dx
		outb	%al,%dx
		xorl	%eax,%eax
		movw	base(,%ecx,2),%dx
		inb		%dx,%al
		movb	%al,%ah
		inb		%dx,%al
		xchgb	%al,%ah
		shrl	$2,%ecx
		shll	%cl,%eax
		popf
		ret
1:		movl	$0xc,_dma_errno
		movl	$-1,%eax
		ret

#int dma_get_status(int channel);
#channel 0-7
#returns the status of the specified channel
#will always be in the form 000r000t where r is the DREQ status bit and t is
#the TC status bit (0x00,0x01,0x10,0x11 are the possible values)
		.align	2
_dma_get_status:
		.set	Channel,	4
		movl	Channel(%esp),%ecx
		cmpl	$7,%ecx
		ja		1f
		movw	status(,%ecx,2),%dx
		xorl	%eax,%eax
		inb		%dx,%al
		andl	$3,%ecx
		shrl	%cl,%eax
		andl	$0x11,%eax
		ret
1:		movl	$0xc,_dma_errno
		movl	$-1,%eax
		ret

		.align	2
# "Extra" messages are for future compatability with the Virtual DMA
# specification.
DMA_E0:	.byte	0
DMA_E1:	.asciz	"Region not in contiguous memory."
DMA_E2:	.asciz	"Region crossed a physical alignment boundary."
DMA_E3:	.asciz	"Unable to lock pages."
DMA_E4:	.asciz	"No buffer available."
DMA_E5:	.asciz	"Region too large for buffer."
DMA_E6:	.asciz	"Buffer currently in use."
DMA_E7:	.asciz	"Invalid memory region."
DMA_E8:	.asciz	"Region was not locked."
DMA_E9:	.asciz	"Number of physical pages greater than table length."
DMA_EA:	.asciz	"Ivalid buffer ID."
DMA_EB:	.asciz	"Copy out of buffer range."
DMA_EC:	.asciz	"Invalid DMA channel number."
		.align	2
		.globl	_dma_errlist
_dma_errlist:
		.long	DMA_E0
		.long	DMA_E1,DMA_E2,DMA_E3,DMA_E4
		.long	DMA_E5,DMA_E6,DMA_E7,DMA_E8
		.long	DMA_E9,DMA_EA,DMA_EB,DMA_EC

.if FOR_DJGPP
		.section .ldata
.else
		.data
.endif
		.globl	_dma_errno
_dma_errno:
		.long	0
