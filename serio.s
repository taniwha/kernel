		.file	"serio.s"

		.set	FOR_DJGPP,	0		## set to 1 if building for djgpp, otherwise
									## assumes building for the kernel

.if !FOR_DJGPP
		.set	KERNEL_DS,	0x10
.endif

		.set	BUFFERLEN,	1024
		.set	FILL,		32		## if <FILL bytes free in rcvbuf, send XOFF
		.set	BRKTIME,	5		## ticks to hold a break

		.set	XOFF,		19
		.set	XON,		17

		.set	DATA_REG,	0
		## Transmit/Receive Buffer  (read/write)
		## Baud Rate Divisor LSB if bit 7 of LCR is set  (read/write)
		.set	IER,		1		## interrupt enable register
		## ³7³6³5³4³3³2³1³0³  2F9, 3F9: Interrupt Enable Register
		##  ³ ³ ³ ³ ³ ³ ³ ÀÄÄÄÄ 1 = enable data available int (and 16550 Timeout)
		##  ³ ³ ³ ³ ³ ³ ÀÄÄÄÄÄ 1 = enable THRE interrupt
		##  ³ ³ ³ ³ ³ ÀÄÄÄÄÄÄ 1 = enable lines status interrupt
		##  ³ ³ ³ ³ ÀÄÄÄÄÄÄÄ 1 = enable modem-status-change interrupt
		##  ÀÄÁÄÁÄÁÄÄÄÄÄÄÄÄ reserved (zero)
		##
		## - 16550 will interrupt if data exists in the FIFO and isn't read
		##   within the time it takes to receive four bytes or if no data is
		##   received within the time it takes to receive four bytes.
		##
		## Baud Rate Divisor MSB if bit 7 of LCR is set  (read/write)
		## - Baud rate divisors can be calculated by taking the oscillating
		##   frequency (1,843,200) and dividing by the quantity of the desired
		##   baud rate times the UART clocking factor (16).  Use the following
		##   formula:
		##   divisor = 1843200 / (BaudRate * 16);
		.set	IIR,		2		## interrupt id register
		## ³7³6³5³4³3³2³1³0³  2FA, 3FA Interrupt ID Register
		##  ³ ³ ³ ³ ³ ³ ³ ÀÄÄÄÄ 1 = no int. pending, 0=int. pending
		##  ³ ³ ³ ³ ³ ÀÄÁÄÄÄÄÄ Interrupt Id bits (see below)
		##  ³ ³ ³ ³ ÀÄÄÄÄÄÄÄÄ 16550  1 = timeout int. pending, 0 for 8250/16450
		##  ³ ³ ÀÄÁÄÄÄÄÄÄÄÄÄ reserved (zero)
		##  ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄ 16550  set to 1 if FIFO queues are enabled
		##
		## Bits
		##  21       Meaning            Priority           To reset
		##  00  modem-status-change      lowest      read MSR
		##  01  transmit-register-empty  low         read IIR / write THR
		##  10  data-available           high        read rec buffer reg
		##  11  line-status              highest     read LSR
		##
		## - interrupt pending flag uses reverse logic, 0 = pending, 1 = none
		## - interrupt will occur if any of the line status bits are set
		## - THRE bit is set when THRE register is emptied into the TSR
		##
		## ³7³6³5³4³3³2³1³0³  2FA, 3FA  FIFO Control Register
		##  ³ ³ ³ ³ ³ ³ ³ ÀÄÄÄÄ 1 = enable clear XMIT and RCVR FIFO queues
		##  ³ ³ ³ ³ ³ ³ ÀÄÄÄÄÄ 1 = clear RCVR FIFO
		##  ³ ³ ³ ³ ³ ÀÄÄÄÄÄÄ 1 = clear XMIT FIFO
		##  ³ ³ ³ ³ ÀÄÄÄÄÄÄÄ 1 = change RXRDY & TXRDY pins from mode 0 to mode 1
		##  ³ ³ ÀÄÁÄÄÄÄÄÄÄÄ reserved (zero)
		##  ÀÄÁÄÄÄÄÄÄÄÄÄÄÄ trigger level for RCVR FIFO interrupt
		##
		## Bits      RCVR FIFO
		##  76     Trigger Level
		##  00        1 byte
		##  01        4 bytes
		##  10        8 bytes
		##  11       14 bytes
		##
		## - Bit 0 must be set in order to write to other FCR bits
		## - Bit 1 when set to 1 the RCVR FIFO is cleared and this bit is reset.
		##   The receiver shift register is not cleared.
		## - Bit 2 when set to 1 the XMIT FIFO is cleared and this bit is reset.
		##   The transmit shift register is not cleared.
		.set	LCR,		3		## line control register
		## ³7³6³5³4³3³2³1³0³  2FB, 3FB  Line Control Register
		##  ³ ³ ³ ³ ³ ³ ÀÄÁÄÄÄÄ word length select bits (see below)
		##  ³ ³ ³ ³ ³ ÀÄÄÄÄÄÄÄ 0 = 1 stop bit, 1 = 1.5 or 2  (see note)
		##  ³ ³ ³ ³ ÀÄÄÄÄÄÄÄÄ 0 = no parity, 1 = parity (PEN)
		##  ³ ³ ³ ÀÄÄÄÄÄÄÄÄÄ 0 = odd parity, 1 = even (EPS)
		##  ³ ³ ÀÄÄÄÄÄÄÄÄÄÄ 0 = parity disabled, 1 = enabled
		##  ³ ÀÄÄÄÄÄÄÄÄÄÄÄ 0 = turn break off, 1 = force spacing break state
		##  ÀÄÄÄÄÄÄÄÄÄÄÄÄ 1 = baud rate divisor (DLAB); 0 = RBR, THR or IER
		.set	W5BITS,		0b00000000
		.set	W6BITS,		0b00000001
		.set	W7BITS,		0b00000010
		.set	W8BITS,		0b00000011
		.set	STOPBITS,	2
		.set	PENABLE,	3
		.set	PNONE,		0b00000000
		.set	PODD,		0b00001000
		.set	PEVEN,		0b00011000
		.set	PMARK,		0b00101000
		.set	PSPACE,		0b00111000
		.set	BREAK,		6
		.set	DLAB,		7
		##
		## Bits
		##  10     Word length bits
		##  00 = 5 bits per character
		##  01 = 6 bits per character
		##  10 = 7 bits per character
		##  11 = 8 bits per character
		##
		## - stop bits = 1.5 for 5 bit words or 2 for 6, 7 or 8 bit words
		## - bit 7 changes the mode of registers 3F8 and 3F9.  If set these
		##   registers become the LSB and MSB of the baud rate divisor.
		##   Otherwise 3F8 is the Transmit/Receive Buffer Register and 3F9 is
		##   the Interrupt Enable Register.
		.set	MCR,		4		## modem control register
		## ³7³6³5³4³3³2³1³0³  2FC, 3FC  Modem Control Register
		##  ³ ³ ³ ³ ³ ³ ³ ÀÄÄÄÄ 1 = activate DTR
		##  ³ ³ ³ ³ ³ ³ ÀÄÄÄÄÄ 1 = activate RTS
		##  ³ ³ ³ ³ ³ ÀÄÄÄÄÄÄ OUT1
		##  ³ ³ ³ ³ ÀÄÄÄÄÄÄÄ OUT2
		##  ³ ³ ³ ÀÄÄÄÄÄÄÄÄ 0 = normal, 1 = loop back test
		##  ÀÄÁÄÁÄÄÄÄÄÄÄÄÄ reserved (zero)
		##
		## - If bit 4 is set, data from the Transmit Shift Register is received
		##   in the Receiver Shift Register.  The SOUT line is set to logic
		##   high, the SIN line and control lines are disconnected.   CTS, DSR,
		##   RI and CD inputs are disconnected.  DTR, RTS, OUT1 and OUT2 are
		##   then connected internally.
		.set	LSR,		5		## line status register
		## ³7³6³5³4³3³2³1³0³  2FD, 3FD Line Status Register
		##  ³ ³ ³ ³ ³ ³ ³ ÀÄÄÄÄ 1 = data ready
		##  ³ ³ ³ ³ ³ ³ ÀÄÄÄÄÄ 1 = overrun error (OE)
		##  ³ ³ ³ ³ ³ ÀÄÄÄÄÄÄ 1 = parity error (PE)
		##  ³ ³ ³ ³ ÀÄÄÄÄÄÄÄ 1 = framing error (FE)
		##  ³ ³ ³ ÀÄÄÄÄÄÄÄÄ 1 = break interrupt  (BI)
		##  ³ ³ ÀÄÄÄÄÄÄÄÄÄ 1 = transmitter holding register empty (THRE)
		##  ³ ÀÄÄÄÄÄÄÄÄÄÄ 1 = transmitter shift register empty (TSRE)
		##  ÀÄÄÄÄÄÄÄÄÄÄÄ 1 = 16550 PE/FE/Break in FIFO queue, 0 for 8250 & 16450
		.set	DR,			0
		.set	OE,			1
		.set	PE,			2
		.set	FE,			3
		.set	BI,			4
		.set	THRE,		5
		##
		## - Bit 0 is set when a byte is placed in the Receiver Buffer Register
		##   and cleared when the byte is read by the CPU (or when the CPU
		##   clears the FIFO for the 16550).  Results in Receive Data Available
		##   Interrupts if enabled.
		## - Bits 1-4 indicate errors and result in Line Status Interrupts
		##   if enabled.
		## - Bit 1 is set when a second byte is received before the byte
		##   in the Receiver Buffer Register is read by the CPU (the 16550 in
		##   FIFO mode sets this bit when the queue is full and the byte in the
		##   Receiver Shift Register hasn't been moved into the queue).  This
		##   bit is reset when the CPU reads the LSR
		## - Bit 2 is set whenever a byte is received that doesn't match the
		##   requested parity.  Reset upon reading the LSR.  (The 16550 maintains
		##   parity information with each byte and sets bit 2 only when the byte
		##   is at the top of the FIFO queue.)
		## - Bit 3 is set when a character is received without proper stop
		##   bits.  Upon detecting a framing error the UART attempts to
		##   resynchronize.  Reset by reading the LSR.  (The 16550 maintains
		##   this information with each byte and sets bit 3 only when the byte
		##   is at the top of the FIFO queue.)
		## - Bit 4 is set when a break condition is sensed (when space is
		##   detected for longer than 1 fullword).  A zero byte is placed in
		##   the Receiver Buffer Register (or 16550 FIFO).  Reset by reading
		##   the LSR.  (The 16550 maintains this information with each byte and
		##   sets bit 4 only when the byte is at the top of the FIFO queue.)
		## - Bit 5 is set when the Transmit Holding Register shifts a byte
		##   into the Transmit Shift Register (or XMIT FIFO queue is empty for
		##   16550) and is cleared when a byte is written to the THR (or the
		##   XMIT FIFO).   Results in Transmit Holding Register Empty interrupts
		##   if enabled.
		## - Bit 6 is set when both the Transmitter Holding Register and the
		##   Transmitter Shift Register are empty. On the 16550, when the XMIT
		##   FIFO and Transmitter Shift Register are empty.
		## - Bit 7 is 16550 specific and indicates there is a byte in the FIFO
		##   queue that was received with a Parity, Framing or Break error.
		.set	MSR,		6		## modem status register
		## ³7³6³5³4³3³2³1³0³  2FE, 3FE Modem Status Register
		##  ³ ³ ³ ³ ³ ³ ³ ÀÄÄÄÄ 1 = DCTS  Delta CTS  (CTS changed)
		##  ³ ³ ³ ³ ³ ³ ÀÄÄÄÄÄ 1 = DDSR  Delta DSR  (DSR changed)
		##  ³ ³ ³ ³ ³ ÀÄÄÄÄÄÄ 1 = RI ring indicator changed
		##  ³ ³ ³ ³ ÀÄÄÄÄÄÄÄ 1 = DDCD  Delta Data Carrier Detect (DCD changed)
		##  ³ ³ ³ ÀÄÄÄÄÄÄÄÄ 1 = CTS
		##  ³ ³ ÀÄÄÄÄÄÄÄÄÄ 1 = DSR
		##  ³ ÀÄÄÄÄÄÄÄÄÄÄ 1 = ring indicator (RI)
		##  ÀÄÄÄÄÄÄÄÄÄÄÄ 1 = receive line signal detect
		##
		## - Bits 0-3 are reset when the CPU reads the MSR
		## - Bit 4 is the Modem Control Register RTS during loopback test
		## - Bit 5 is the Modem Control Register DTR during loopback test
		## - Bit 6 is the Modem Control Register OUT1 during loopback test
		## - Bit 7 is the Modem Control Register OUT2 during loopback test

		.set	sentxoff,	0
		.set	sendxoff,	1
		.set	sendxon,	2
		.set	gotxoff,	3


.if FOR_DJGPP
		.section	.ltext
.else
		.text
.endif
## ISR for UART interrupt
		.align	2
comint:
		## gas seg override bug does not rear it's ugly head here because
		## we're not moving between %eax and direcect memory
		## (eg movl foo,%eax ) which uses the short form of mov to/from the
		## ax register (any size)
		movl	portIIR(%esi),%edx	## get the comm port interrupt
									## %cs and %ds are aliased
		inb		%dx,%al				## identification register and read it
		## test whether it was the commport generating the interrupt. If it
		## was, bit 0 will be clear (and hence the carry flag), and move the
		## interrupt identification bits (1,2) into %al for the indexed
		## function call later.
		shrb	$1,%al				## NC=interrupt, CY=no interrupt
		jnc		1f					## (must chain if carry set as the
									## interrupt may be shared)
		## irq was not for us so restore %eax and %edx and then jump to the
		## next isr for this interrupt
		ret
1:		andl	$3,%eax					## incase 16550
#		movzbl	%al,%eax
		call	*commFuncTable(,%eax,4)	## call the appropriate comm routine
										## IIR==0 (%al==0) -> msr
										## IIR==2 (%al==1) -> xmit
										## IIR==4 (%al==2) -> rcv
										## IIR==6 (%al==3) -> linestat
		## reload the IIR register to see if there is anything more to do to
		## clear the comm port rather than generating unnecessary interrupts
		movl	portIIR(%esi),%edx	## point to the interrupt identification
		inb		%dx,%al				## register and read it
		shrb	$1,%al				## NC=interrupt, CY=no interrupt
		jnc		1b					## (continue if carry clear (another
									## pending interrupt))
3:		ret

		.align	2
comisr0:pushl	%esi
		movl	$0x00,%esi
		jmp		comCommon
		.align	2
comisr1:pushl	%esi
		movl	$0x01,%esi
		jmp		comCommon
		.align	2
comisr2:pushl	%esi
		movl	$0x02,%esi
		jmp		comCommon
		.align	2
comisr3:pushl	%esi
		movl	$0x03,%esi
		jmp		comCommon
		.align	2
comisr4:pushl	%esi
		movl	$0x04,%esi
		jmp		comCommon
		.align	2
comisr5:pushl	%esi
		movl	$0x05,%esi
		jmp		comCommon
		.align	2
comisr6:pushl	%esi
		movl	$0x06,%esi
		jmp		comCommon
		.align	2
comisr7:pushl	%esi
		movl	$0x07,%esi
		jmp		comCommon
		.align	2
comisr8:pushl	%esi
		movl	$0x08,%esi
		jmp		comCommon
		.align	2
comisr9:pushl	%esi
		movl	$0x09,%esi
		jmp		comCommon
		.align	2
comisrA:pushl	%esi
		movl	$0x0A,%esi
		jmp		comCommon
		.align	2
comisrB:pushl	%esi
		movl	$0x0B,%esi
		jmp		comCommon
		.align	2
comisrC:pushl	%esi
		movl	$0x0C,%esi
		jmp		comCommon
		.align	2
comisrD:pushl	%esi
		movl	$0x0D,%esi
		jmp		comCommon
		.align	2
comisrE:pushl	%esi
		movl	$0x0E,%esi
		jmp		comCommon
		.align	2
comisrF:pushl	%esi
		movl	$0x0F,%esi
		jmp		comCommon

		.align	2
comCommon:
		pushl	%edx
		pushl	%eax
		pushl	%ds
		## gas seg override bug does not rear it's ugly head here because
		## we're not moving between %eax and direcect memory
		## (eg movl foo,%eax ) which uses the short form of mov to/from the
		## ax register (any size)
.if FOR_DJGPP
		movw	%cs:ourds,%ds
.else
		movw	$KERNEL_DS,%ax
		movw	%ax,%ds
.endif
##		pushl	%esi
		movl	%esi,%eax
		btl		$3,%esi				## test bit 3 (indicates irq 8-15)
		sti
		movb	$0x20,%al			## non-specific end of interrupt
		jnc		3f
		outb	%al,$0xa0
3:		outb	%al,$0x20

4:		movl	portData(,%esi,4),%esi
		jmp		2f
1:		call	comint
		movl	nextPort(%esi),%esi
2:		orl		%esi,%esi
		jnz		1b
		## clear the intterupt from the PIC
##		popl	%eax
##		btrw	$3,%ax				## test and clear bit 3 (indicates irq 8-15)
##		jc		3f
##		orb		$0b01100000,%al		## specific end of interrupt
##		outb	%al,$0x20
##		jmp		4f
##3:		orb		$0b01100000,%al		## specific end of interrupt
##		outb	%al,$0xa0
##		movb	$0b01100010,%al		## specific end of interrupt(2)
##		outb	%al,$0x20
##		## restore the registers and return to the interrupted code
4:		popl	%ds
		popl	%eax
		popl	%edx
		popl	%esi
		iret

## Handle Modem Status interrupts
		.align	2
msr:
		movl	portMSR(%esi),%edx
		inb		%dx,%al
		movb	%al,_sio_modemstat(%esi)
		ret

## Handle Line Status interrupts
		.align	2
linestat:
		movl	portLSR(%esi),%edx
		inb		%dx,%al
		andb	$0b00011110,%al		## clear out the irrelevant bits
		movb	%al,_sio_linestat(%esi)
		btw		$BI,%ax				## test for break
		jnc		1f
		movb	$1,_sio_break(%esi)
1:		ret

## increment buffer pointer circularly
## %ebx holds buffer pointer and %ecx holds the offset to increment by
		.align	2
circadd:
		addl	%ecx,%ebx
1:		cmpl	$BUFFERLEN,%ebx
		jb		2f
		subl	$BUFFERLEN,%ebx
		jmp		1b
2:		ret

## Attempt to transmit character
		.align	2
xmit:
		## test the 
1:		movl	portLSR(%esi),%edx
		inb		%dx,%al
		btw		$THRE,%ax			## tx register empty
		jc		1f
		jmp		Lxmit_exit			## tx register not empty, false alarm
		## check to see if an xoff should be sent
1:		btrl	$sendxoff,flags(%esi)
		jnc		2f
		## if so, send the 'xoff' and set the 'sent xoff' flag after clearing
		## the 'send xoff' flag
		movb	$XOFF,%al
		movl	portTHR(%esi),%edx
		outb	%al,%dx
		btsl	$sentxoff,flags(%esi)
		jmp		Lxmit_exit			## nothing more to do here
		## check to see if an xon should be sent
2:		btrl	$sendxon,flags(%esi)
		jnc		3f
		## if so, send the 'xon' and clear the 'sent xoff' flag after clearing
		## the 'send xon' flag
		movb	$XON,%al
		movl	portTHR(%esi),%edx
		outb	%al,%dx
		btrl	$sentxoff,flags(%esi)
		jmp		Lxmit_exit			## nothing more to do here
		## save %ebx and %ecx as we use these registers
3:		pushl	%ebx
		pushl	%ecx
		## if we're doing software flow control and an xoff was received, or
		## there are no more characters to be sent, then exit
		testb	$0xff,_sio_doxoff(%esi)	## s/w flow control
		jz		4f
		btl		$gotxoff,flags(%esi)## an xoff was received
		jc		5f
4:		movl	xmittail(%esi),%ebx	## load the tail pointer into %ebx
		cmpl	xmithead(%esi),%ebx	## is the buffer empty
		jne		6f					## not equal means chars in buffer
		## we cannot send another character right now, so restore %ecx and
		## %ebx and return
5:		popl	%ecx
		popl	%ebx
		jmp		Lxmit_exit
		## get a char from the transmit buffer
6:		movb	xmitbuf(%esi,%ebx),%al	## %ebx loaded previously
		## point to the next character in the transmit buffer
		movl	$1,%ecx				## xmittail++
		call	circadd				## with wrap at buffer size
		movl	%ebx,xmittail(%esi)
		## send the character
		movl	portTHR(%esi),%edx
		outb	%al,%dx
		incl	_sio_chars_sent(%esi)
		## restor %ecx and %ebx and exit
		popl	%ecx
		popl	%ebx
Lxmit_exit:
		ret

		.align	2
testbuf:
## tests for buffer overflow or underflow
## call with:
##	%eax = head (overflow) tail (underflow)
##	%edx = tail (overflow) head (underflow)
##	%ecx = offset for circadd
## 
## clobbers %ebx (holds result of circadd, initially assigned from %eax)
## carry set=buffer overflow/underflow
## carry clear=buffer ok
## base logic:
##	(oflow) head<tail && (temp>=tail || temp<head) || temp>=tail && temp<head
##	(uflow) tail<head && (temp>=head || temp<tail) || temp>=head && temp<tail
## comments assume testing for overflow
		movl	%eax,%ebx
		call	circadd
		cmpl	%edx,%eax		## head < tail
		jae		1f				## jump if tail >= head (sc on failure &&)
		cmpl	%edx,%ebx		## temp >= tail
		jae		2f				## jump if temp >= tail (sc on success ||)
		cmpl	%eax,%ebx		## temp < head
		jb		2f				## jump if temp < head  (sc on success ||)
		## previous conditions failed
1:		cmpl	%edx,%ebx		## temp >= tail
		jb		3f				## jump if tail >= temp (sc on failure &&)
		cmpl	%eax,%ebx		## temp < head
		jae		3f				## jump if head >= tail (sc on failure &&)
		## buffer overflow (or underflow if %eax and %edx reversed)
2:		stc
		ret
		## buffer does not overflow (or underflow if %eax and %edx reversed)
3:		clc
		ret

##Attempt to receive a character
		.align	2
rcv:
		movl	portLSR(%esi),%edx
		inb		%dx,%al
		btw		$DR,%ax				## rx register full
		jc		1f
		ret							## rx register not full, false alarm
1:		pushl	%ebx				## save %ebx and %ecx
		pushl	%ecx
		movl	portRBR(%esi),%edx
		inb		%dx,%al				## read in the received character. always has
									## to be done to clear the uart interrupt
									## register and prevent overrun errors
		## tuck the char away in the receive buffer. There is always a hole
		## that is never used, even if the buffer is full because head points
		## to the next available slot, and if the hole was not available, head
		## would wind up equalling tail, which implies an empty buffer
		movl	rcvhead(%esi),%ebx
		movb	%al,rcvbuf(%esi,%ebx)
		## always flag XOFF and XON for flow control (xmit chooses weather or
		## not to ignore them) even if the buffer is full
		cmpb	$XOFF,%al
		jne		7f
		btsl	$gotxoff,flags(%esi)
		jmp		8f
7:		cmpb	$XON,%al
		jne		8f
		btrl	$gotxoff,flags(%esi)
8:		## test if receive buffer is full
		movl	rcvhead(%esi),%eax	## testing for overflow so load head into %eax
		movl	rcvtail(%esi),%edx	## and tail into %edx
		movl	$1,%ecx				## one byte to check
		call	testbuf				## cy=full, nc=not full
		jnc		2f
		## buffer overflow
		movl	$1,_sio_error(%esi)
		incl	_sio_errct(%esi)
		popl	%ecx
		popl	%ebx
		ret
		## the buffer is not full, so save the new head pointer (%ebx was set
		## up by the call to testbuf).
2:		movl	%ebx,rcvhead(%esi)
		## test if less than 'FILL' bytes free in receive buffer (%eax and
		## %edx are already initialized to rcvhead and rcvtail, respectively)
		movl	$FILL,%ecx		## FILL bytes to check
		call	testbuf			## cy=full, nc=not full
		jnc		4f
		testb	$0xff,_sio_doxoff(%esi)
		jz		4f
		## send an xoff character
		btsl	$sendxoff,flags(%esi)
		## if the transmit register is empty, send the xoff char, otherwise
		## xmit will do nothing this time but xoff will be sent as soon as the
		## character in the register has been sent.
		call	xmit
4:		popl	%ecx
		popl	%ebx
		ret

		.align	2
disable:
# NOTE!: Under Windows 3.xx this will not work! (Unless our cpl is bumped from 3)
		pushfl
		cli
		popl	%eax
		ret

		.align	2
enable:
# NOTE!: Under Windows 3.xx this will not work! (Unless our cpl is bumped from 3)
		pushl	%eax
		popfl
		ret

## User interface routines
## send a character. returns 0 if successful, 1 if transmit buffer is full
		.align	2
_sio_put:.globl	_sio_put
		call	disable
		pushl	%eax
		pushl	%esi
		pushl	%ebx
		movl	16(%esp),%esi
		movb	20(%esp),%cl    	## get the character to be sent
		movl	xmithead(%esi),%eax	## testing for overflow so load head into %eax
		movl	xmittail(%esi),%edx	## and tail into %edx
		movb	%cl,xmitbuf(%esi,%eax)	## put charachter to be send in the buffer
		movl	$1,%ecx				## one byte to check
		call	testbuf
		jc		1f					## no carry if no overflow
		movl	%ebx,xmithead(%esi)	## %ebx has been initialized by testbuf above
		## Call xmit in case transmitter has been idle for a while. If
		## transmitter is busy, xmit won't do anything
		call	xmit			## give the transmitter a kick

		movl	$0,%ecx
		jmp		2f
1:		movl	$1,%ecx
2:		popl	%ebx
		popl	%esi
		popl	%eax
		call	enable
		movl	%ecx,%eax
		ret

		.align	2
_sio_write:.globl	_sio_write
		call	disable
		pushl	%eax
		pushl	%ebx
		pushl	%ebp
		pushl	%edi
		movl	24(%esp),%ebp
		movl	28(%esp),%esi
		movl	32(%esp),%edi
		orl		%edi,%edi
		jz		3f
		movl	xmithead(%ebp),%eax	## testing for overflow so load head into %eax
		movl	xmittail(%ebp),%edx	## and tail into %edx
1:		movb	(%esi),%cl
		movb	%cl,xmitbuf(%ebp,%eax)	## put charachter to be send in the buffer
		movl	$1,%ecx				## one byte to check
		call	testbuf
		jc		2f					## no carry if no overflow
		movl	%ebx,%eax			## %ebx has been initialized by testbuf above
		incl	%esi
		decl	%edi
		jnz		1b
2:		movl	%ebx,xmithead(%ebp)
		call	xmit
3:		movl	32(%esp),%ecx
		subl	%edi,%ecx
		popl	%edi
		popl	%esi
		popl	%ebx
		popl	%eax
		call	enable
		movl	%ecx,%eax
		ret

## Check for character available (1 means char ready)
		.align	2
_sio_charready:.globl	_sio_charready
		call	disable
		pushl	%eax
		pushl	%esi
		movl	12(%esp),%esi
		## if rcvtail == rcv head then there are no characters in the buffer
		movl	rcvtail(%esi),%eax
		cmpl	%eax,rcvhead(%esi)
		setne	%al
		movzbl	%al,%ecx
		popl	%esi
		popl	%eax
		call	enable
		movl	%ecx,%eax
		ret

## Wait for character and return it. returns -1 for break
		.align	2
_sio_get:.globl	_sio_get
		pushl	%esi
		movl	8(%esp),%esi
		pushl	%esi
		##	while (rcvtail==rcvhead) {
1:		call	_sio_charready
		andl	%eax,%eax
		jnz		2f
		##		if (_sio_brkmode
		testb	$0xff,_sio_brkmode(%esi)
		jz		1b
		##						&& _sio_break) {
		testb	$0xff,_sio_break(%esi)
		jz		1b
		movl	$0,_sio_break(%esi)
		##			return -1
		movl	$-1,%eax
		addl	$4,%esp
		popl	%esi
		ret
		##		}
		##	}
		## a character is available (%ebx alreadly loaded with rcvtail)
2:		addl	$4,%esp
		call	disable
		pushl	%eax
		pushl	%ebx			## save %ebx (for gcc)
		movl	rcvtail(%esi),%ebx
		movb	rcvbuf(%esi,%ebx),%al
		movzbl	%al,%eax
		movl	$1,%ecx
		call	circadd
		movl	%ebx,rcvtail(%esi)
		pushl	%eax			## save received character on the stack
		## if an xoff was not sent then there is no need to check weather or
		## not to send an xon
		btl		$sentxoff,flags(%esi)
		jnc		3f
		## test if less than FILL bytes remaining in buffer
		movl	rcvhead(%esi),%edx	## testing for underflow so load head into
		movl	rcvtail(%esi),%eax	## %edx and tail into %eax
		movl	$FILL,%ecx		## FILL bytes to check
		call	testbuf
		jnc		3f				## more than FILL chars remaining if no carry
		## send an xon character
		btsl	$sendxon,flags(%esi)
		## if the transmit register is empty, send the xon char, otherwise
		## xmit will do nothing this time but xon will be sent as soon as the
		## character in the register has been sent.
		call	xmit
3:		popl	%ecx			## restore the received char from the stack
		popl	%ebx			## restore %ebx (for gcc)
		popl	%eax
		call	enable
		popl	%esi
		movl	%ecx,%eax
		ret

## Send a break
		.align	2
_sio_sendbreak:.globl _sio_sendbreak
		pushl	%esi
		movl	8(%esp),%esi
		movl	12(%esp),%ecx	## number of character cells to hold break
		## wait until the transmitter is empty
		movl	portLSR(%esi),%edx
1:		inb		%dx,%al
		btw		$THRE,%ax
		jnc		1b
		## set the break control bit of the line control register
		movl	portLCR(%esi),%edx
		inb		%dx,%al
		btsw	$BREAK,%ax
		outb	%al,%dx
		## send a character (anything will do(?)) for timing purposes
2:		movl	portTHR(%esi),%edx
		outb	%al,%dx
		## wait until the transmitter is empty (the bogus character has been
		## sent)
		movl	portLSR(%esi),%edx
3:		inb		%dx,%al
		btw		$THRE,%ax
		jnc		3b
		decl	%ecx			## ecx holds the number of `characters' to
		jne		2b				## send while holding the break condition
		## clear the break condition
		movl	portLCR(%esi),%edx
		inb		%dx,%al
		btrw	$BREAK,%ax
		outb	%al,%dx
		popl	%esi
		ret

# the following two functions provide access to the divisor latch registers
# in a convenient fassion.  HOWEVER: the program must do the translation
# to/from baud as these functions DO NOT.  This allows the program to set
# strange baud rates or control the allowable rates rather than putting
# restrictions on the program.
		.align	2
_sio_setspeed:	.globl	_sio_setspeed
		movl	4(%esp),%ecx
		## sets the divisor latch to the value passed on the stack.  DOES NOT
		## convert from baud.
		movl	portLCR(%ecx),%edx
		inb		%dx,%al
		btsw	$DLAB,%ax		## access the divisor latch registers
		outb	%al,%dx
		movw	8(%esp),%ax
		movl	portDLL(%ecx),%edx
		outb	%al,%dx			## set LSB of divisor
		movl	portDLM(%ecx),%edx
		movb	%ah,%al
		outb	%al,%dx			## set MSB of divisor
		movl	portLCR(%ecx),%edx
		inb		%dx,%al
		btrw	$DLAB,%ax		## access data/interrupt enable registers
		outb	%al,%dx
		ret

		.align	2
_sio_getspeed:	.globl	_sio_getspeed
		movl	4(%esp),%ecx
		## gets the divisor latch.  DOES NOT convert to baud.
		movl	portLCR(%ecx),%edx
		inb		%dx,%al
		btsw	$DLAB,%ax		## access the divisor latch registers
		outb	%al,%dx
		movl	portDLM(%ecx),%edx
		inb		%dx,%al			## get MSB of divisor
		movl	portDLL(%ecx),%edx
		movb	%al,%ah
		inb		%dx,%al			## get LSB of divisor
		pushl	%eax
		movl	portLCR(%ecx),%edx
		inb		%dx,%al
		btrw	$DLAB,%ax		## access data/interrupt enable registers
		outb	%al,%dx
		popl	%eax
		ret

# the following two functions provide direct access to the modem control
# register. NOT RECOMMENDED but provided for when direct access is required
		.align	2
_sio_setmcr:	.globl	_sio_setmcr
		movl	4(%esp),%ecx
		movl	portMCR(%ecx),%edx
		movb	8(%esp),%al
		outb	%al,%dx
		ret

		.align	2
_sio_getmcr:	.globl	_sio_getmcr
		movl	4(%esp),%ecx
		movl	portMCR(%ecx),%edx
		xorl	%eax,%eax
		inb		%dx,%al
		ret

# the following two functions provide direct access to the line control
# register. NOT RECOMMENDED but provided for when direct access is required
		.align	2
_sio_setlcr:	.globl	_sio_setlcr
		movl	4(%esp),%ecx
		movl	portMCR(%ecx),%edx
		movb	8(%esp),%al
		outb	%al,%dx
		ret

		.align	2
_sio_getlcr:	.globl	_sio_getlcr
		movl	4(%esp),%ecx
		movl	portLCR(%ecx),%edx
		xorl	%eax,%eax
		inb		%dx,%al
		ret

# set type communications paramters (but not the baud rate)
		.align	2
_sio_setparms:	.globl	_sio_setparms
		movl	4(%esp),%ecx
		## assumes enums defined in serio.h were used.  the enums set the
		## correct bits in the control byte and can be just or'ed together
		## and output to the line control regitster.
		movb	8(%esp),%al		# bits per word
		orb		12(%esp),%al	# parity
		orb		16(%esp),%al	# stop bits
		movl	portLCR(%ecx),%edx
		andb	$0b00111111,%al
		outb	%al,%dx
		ret

## The .text directive is to get around a bug in gas (ld?) which causes relocations
## in .ltext to go funny.
.if FOR_DJGPP
		.text
.else
.endif
## sets up the UART register addresses. separate variables are used for each
## register for both readability and speed(? removes need for adding an offset
## to dx, movl portLCR,%edx; inb %dx,%al will read the line control register)
		.align	2
sio_setbase:
		movl	%edx,basead(%esi)
		movl	%edx,portDLL(%esi)
		movl	%edx,portTHR(%esi)
		movl	%edx,portRBR(%esi)
		incl	%edx
		movl	%edx,portDLM(%esi)
		movl	%edx,portIER(%esi)
		incl	%edx
		movl	%edx,portIIR(%esi)
		incl	%edx
		movl	%edx,portLCR(%esi)
		incl	%edx
		movl	%edx,portMCR(%esi)
		incl	%edx
		movl	%edx,portLSR(%esi)
		incl	%edx
		movl	%edx,portMSR(%esi)
		
		movl	%ebx,irqnum(%esi)
		xorl	%edx,%edx
		movl	%edx,xmithead(%esi)
		movl	%edx,xmittail(%esi)
		movl	%edx,rcvhead(%esi)
		movl	%edx,rcvtail(%esi)
		movl	%edx,flags(%esi)
		movl	%edx,_sio_error(%esi)
		movl	%edx,_sio_errct(%esi)
		movl	%edx,_sio_chars_sent(%esi)
		movl	%edx,_sio_break(%esi)
		movl	%edx,_sio_linestat(%esi)
		movl	%edx,_sio_modemstat(%esi)
		movl	%edx,_sio_doxoff(%esi)
		movl	%edx,_sio_brkmode(%esi)
		ret

# hooks the interrupt associated with the IRQ number in ebx. saves previous
# vector for chaining and restoring at module cleanup.
# also sets the IRQ masks for the PICs
sio_hookinterrupt:
.if FOR_DJGPP
		pushl	%ebx
		# save old interrupt vector
		# cx:edx address (sel:offs)
		movzbl	irqints(%ebx),%ebx
		movw	$0x0204,%ax
		int		$0x31
		movl	(%esp),%eax
		movl	%edx,oldirq+old_offs(,%eax,8)
		movw	%cx,oldirq+old_sel(,%eax,8)

		# hook interrupt
		# cx:edx address (sel:offs)
		movw	%cs,%cx
		movl	comisr(,%eax,4),%edx
		movw	$0x0205,%ax
		int		$0x31
		popl	%ebx
.else
		movl	%ebx,%eax
		call	get_irq_vector
		movl	%eax,oldirq+old_offs(,%ebx,8)
		movw	%dx,oldirq+old_sel(,%ebx,8)

		movl	%ebx,%eax
		movw	%cs,%cx
		movl	comisr(,%eax,4),%edx
		call	set_irq_vector
.endif
		
		inb		$0xa1,%al
		movb	%al,%ah
		inb		$0x21,%al
		movw	%ax,%cx
		andw	irqemasks(,%ebx,2),%ax	# calculate value to enable this irq
		andw	irqdmasks(,%ebx,2),%cx	# calculate value to disable this irq
										# for cleanup
		jnz		1f
		btrl	%ebx,oldmasks
1:		outb	%al,$0x21
		movb	%ah,%al
		outb	%al,$0xa1
		ret

# restore the interrupt vector for the irq in ebx and restores the PIC int
# enable mask to what it was at startup (this allows the program to hook the
# mouse comport for its own uses without killing the mouse on exit)
sio_unhookinterrupt:
		pushl	%ebx
		inb		$0xa1,%al
		movb	%al,%ah
		inb		$0x21,%al
		btsl	%ebx,oldmasks
		jnc		1f
		orw		irqdmasks(,%ebx,2),%ax	# restore the PIC mask for this irq to what it
										# was when this module was initialize
1:		outb	%al,$0x21
		movb	%ah,%al
		outb	%al,$0xa1
		# unhook interrupt
		# cx:edx address (sel:offs)
		movw	oldirq+old_sel(,%ebx,8),%cx
		movl	oldirq+old_offs(,%ebx,8),%edx
.if FOR_DJGPP
		movzbl	irqints(%ebx),%ebx
		movw	$0x0205,%ax
		int		$0x31
.else
		movl	%ebx,%eax
		call	set_irq_vector
.endif
		popl	%ebx
		ret

		.align	2
sio_init_port:
		movl	portMCR(%esi),%edx
		inb		%dx,%al
		movb	%al,%ah
		andb	$~0b1000,%al
		outb	%al,%dx
##
##		movl	$1000000,%ecx
##1:		nop
##		jmp		0
##		loop	1b
##

		movl	portIER(%esi),%edx
		inb		%dx,%al
		movw	%ax,cntlRegs(%esi)
		movb	$0b1111,%al
		outb	%al,%dx

		xorl	%eax,%eax

2:		movl	portIIR(%esi),%edx
		inb		%dx,%al
		shrb	$1,%al
		jc		3f
		jmp		*init_port_case(,%eax,4)
		.align	2
init_port_case:
		.long	ip_msr
		.long	ip_tbr
		.long	ip_rbr
		.long	ip_lsr
		.align	2
ip_lsr:
		movl	portLSR(%esi),%edx
		inb		%dx,%al
		jmp		2b
		.align	2
ip_rbr:
		movl	portLSR(%esi),%edx
		inb		%dx,%al
		movl	portRBR(%esi),%edx
		inb		%dx,%al
		jmp		2b
		.align	2
ip_tbr:
		movl	portLSR(%esi),%edx
		inb		%dx,%al
		jmp		2b
		.align	2
ip_msr:
		movl	portMSR(%esi),%edx
		inb		%dx,%al
		jmp		2b
3:
		## clear the interrupt from the PIC
		movb	irqnum(%esi),%al
		btrw	$3,%ax				## test and clear bit 3 (indicates irq 8-15)
		jc		1f
		orb		$0b01100000,%al		## specific end of interrupt
		outb	%al,$0x20
		jmp		2f
1:		orb		$0b01100000,%al		## specific end of interrupt
		outb	%al,$0xa0
		movb	$0b01100010,%al		## specific end of interrupt(2)
		outb	%al,$0x20
2:
		movl	portMCR(%esi),%edx
		inb		%dx,%al
		orb		$0b01011,%al
		outb	%al,%dx
		ret

		.align	2
_sio_openport:	.globl	_sio_openport
		.set	BaseAddress,8
		.set	Irq,		12
		pushl	%ebp
		movl	%esp,%ebp
		pushl	%ebx
		pushl	%esi

.if FOR_DJGPP
		movw	___djgpp_ds_alias,%eax
		movw	%eax,ourds
.endif

		xorl	%eax,%eax
		movl	Irq(%ebp),%ebx
		cmpl	$15,%ebx
		ja		3f

		## allocate space for the port info block
		movl	freePorts,%esi
		testl	%esi,%esi
		jnz		1f
		## no free ports, so allocate one from the heap
		pushl	$PORTSIZE
.if FOR_DJGPP
		call	_malloc
.else
		call	_kmalloc
.endif
		addl	$4,%esp
		testl	%eax,%eax
		jz		4f
		movl	%eax,%esi
.if FOR_DJGPP
		## lock it down
		pushl	$PORTSIZE
		pushl	%esi
		call	__go32_dpmi_lock_data
		addl	$8,%esp
		testl	%eax,%eax
		jz		1f
		## agh, can't lock the memory, so free the chunk and get out of here
		pushl	%esi
		call	_free
		xorl	%eax,%eax
		jmp		3f
.else
		## `kmalloc'ed memory is always locked, so no need to lock it (no way
		## either)
.endif

1:		movl	BaseAddress(%ebp),%edx
		call	sio_setbase

		## link it into the chain of port blocks (for interrupt sharring)
		movl	portData(,%ebx,4),%eax
		movl	%eax,nextPort(%esi)
		movl	%esi,portData(,%ebx,4)
		testl	%eax,%eax
		jnz		2f
		## if there are any ports in the chain, then the interrupt has already
		## been hooked, otherwise it must be hooked.
		call	sio_hookinterrupt
2:		call	sio_init_port
		movl	%esi,%eax

3:		popl	%esi
		popl	%ebx
		popl	%ebp
		ret

_sio_closeport:	.globl	_sio_closeport
		.set	PortInfo,	8
		pushl	%ebp
		movl	%esp,%ebp
		pushl	%ebx
		pushl	%esi
		pushl	%edi

		movl	PortInfo(%ebp),%esi
		testl	%esi,%esi
		jz		5f

##		movl	$0x0f,%ebx
		movl	irqnum(%esi),%ebx
1:		leal	portData-nextPort(,%ebx,4),%edi
2:		cmpl	nextPort(%edi),%esi
		je		3f
		movl	nextPort(%edi),%edi
		testl	%edi,%edi
		jnz		2b
##		decl	%ebx
##		jns		1b
		jmp		5f

3:		movl	portIER(%esi),%edx
		movw	cntlRegs(%esi),%ax
		outb	%al,%dx
		movl	portMCR(%esi),%edx
		movb	%ah,%al
		outb	%al,%dx

		movl	nextPort(%esi),%edx
		movl	%edx,nextPort(%edi)

		cmpl	$0,portData(,%ebx,4)
		jne		4f
		call	sio_unhookinterrupt

4:		movl	freePorts,%eax
		movl	%eax,nextPort(%esi)
		movl	%esi,freePorts

5:		popl	%edi
		popl	%esi
		popl	%ebx
		popl	%ebp
		ret

.if FOR_DJGPP
		.section	.ldata
.else
		.data
.endif

commFuncTable:
		.long	msr,		xmit,		rcv,		linestat
		## porter to free (closed) ports. Allows recyling of allocated and
		## locked memory.
freePorts:
		.long	0
		## table of open ports (each entry is a linked list of shared ports)
portData:
		.fill	16,4,0
		## lookup table for irq to isr translation
comisr:
		.long	comisr0,	comisr1,	comisr2,	comisr3
		.long	comisr4,	comisr5,	comisr6,	comisr7
		.long	comisr8,	comisr9,	comisrA,	comisrB
		.long	comisrC,	comisrD,	comisrE,	comisrF
oldirq:	.fill	16,8,0
		.set	old_offs,	0		## offset
		.set	old_sel,	4		## selector
oldmasks:.word	0xfffb				## irq 2 is assumed to be always enabled
									## as it is used to chain the second icu
.if FOR_DJGPP
## The following definitions (up to the definition of the irq bits) are only
## needed when building for djgpp as the kernal provides better facilities for
## such things (ring 0 code is kewl!)
ourds:	.word	0


irqints:
		## irq to interrupt translation.  Assumes standard setup of PICs
		## (guaranteed(?) under dpmi)
		.byte	0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
		.byte	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77
.endif
		.align	2
irqemasks:
		## masks for enableing PIC interrupts. IRQ 2 is automatically enabled
		## whenever IRQ 8-15 is enabled. Recomended useage:
		##		inb		$0xa1,%al				# read slave int enable mask
		##		movb	%al,%ah					# irq8-15 in bits 8-15 of ax
		##		inb		$0x21,%al				# read master int enable mask
		##										# irq0-7 in bits 0-7 of ax
		##		andw	irqemasks(,%ebx,2),%ax	# ebx holds irq number
		##		outb	%al,$0x21				# write master int enable mask
		##		movb	%ah,$al
		##		outb	%al,$0xa1				# write slave int enable mask
		##
		## related info: with the 8259 PIC a 1 in the interrupt enable mask
		## DISABLES the corresponding irq (ie a 1 in bit 3 disables irq 3, or
		## 11 if the PIC is the slave) and a 0 ENABLES the irq.
		.word	0b1111111111111110
		.word	0b1111111111111101
		.word	0b1111111111111011
		.word	0b1111111111110111
		.word	0b1111111111101111
		.word	0b1111111111011111
		.word	0b1111111110111111
		.word	0b1111111101111111
		.word	0b1111111011111011
		.word	0b1111110111111011
		.word	0b1111101111111011
		.word	0b1111011111111011
		.word	0b1110111111111011
		.word	0b1101111111111011
		.word	0b1011111111111011
		.word	0b0111111111111011
irqdmasks:
		## masks for disabling PIC interrupts. IRQ2 is not disabled by
		## disabling IRQ's 8-15 as this would cause the other IRQ's in
		## this range to be disabled, which would be a BAD THING (would kill
		## the hard drive, although I think the bios always ensures the HD
		## IRQ is enabled every time it's needed).
		.word	0b0000000000000001
		.word	0b0000000000000010
		.word	0b0000000000000100
		.word	0b0000000000001000
		.word	0b0000000000010000
		.word	0b0000000000100000
		.word	0b0000000001000000
		.word	0b0000000010000000
		.word	0b0000000100000000
		.word	0b0000001000000000
		.word	0b0000010000000000
		.word	0b0000100000000000
		.word	0b0001000000000000
		.word	0b0010000000000000
		.word	0b0100000000000000
		.word	0b1000000000000000
## global variables of interest to user's program
## error words
		.set	_sio_error,		0	## rx buffer overflowed
		.set	_sio_errct,		4	## number of rx buffer errors
		.set	_sio_chars_sent,8	## number of characters transmitted
		.set	_sio_break,		12	## set to 1 when break occurs
		.set	_sio_linestat,	16	## line status
		.set	_sio_modemstat,	20	## modem status
		.set	_sio_doxoff,	24	## set xon/xoff mode on
		.set	_sio_brkmode,	28	## if 1, _sio_get() returns -1 if break occured
## private data
		.set	nextPort,		32
		.set	basead,			36	## base address of UART I/O
		.set	portDLL,		36	## divisor latch LSB
		.set	portTHR,		36	## transmit holding register
		.set	portRBR,		36	## receiver buffer register
		.set	portDLM,		40	## divisor latch MSB
		.set	portIER,		40	## interrupt enable register
		.set	portIIR,		44	## interrupt identification register
		.set	portLCR,		48	## line control register
		.set	portMCR,		52	## modem control register
		.set	portLSR,		56	## line status register
		.set	portMSR,		60	## modem status register
		.set	cntlRegs,		64	## initial values of mcr and ier
		.set	irqnum,			68  ## the irq line this port uses
		.set	xmithead,		72  ## tx buffer head pointer
		.set	xmittail,		76	## tx buffer tail pointer
		.set	rcvhead,		80	## rx buffer head pointer
		.set	rcvtail,		84	## rx buffer tail pointer
		.set	flags,			88	## state flags (got xoff etc)
## Define two circular buffers
		.set	xmitbuf,		flags+4				## tx buffer
		.set	rcvbuf,			xmitbuf+BUFFERLEN	## rx buffer
		.set	PORTSIZE,		rcvbuf+BUFFERLEN	## size of this structure
