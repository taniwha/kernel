		.file	"keybdio.s"

		.set	S_RShift,	0b0000000000000001
		.set	S_LShift,	0b0000000000000010
		.set	S_Shift,	0b0000000000000011
		.set	S_Ctrl,		0b0000000000000100
		.set	S_Alt,		0b0000000000001000
		.set	S_ScrlLock,	0b0000000000010000
		.set	S_NumLock,	0b0000000000100000
		.set	S_CapsLock,	0b0000000001000000
		.set	Leds,		0b0000000001110000
		.set	LedsShift,	4
		.set	S_Insert,	0b0000000010000000
		.set	S_LCtrl,	0b0000000100000000
		.set	S_LAlt,		0b0000001000000000
		.set	S_RCtrl,	0b0000010000000000
		.set	S_RAlt,		0b0000100000000000
		.set	S_ctrl,		0b0000010100000000
		.set	S_alt,		0b0000101000000000
		.set	S_scrllock,	0b0001000000000000
		.set	S_numlock,	0b0010000000000000
		.set	S_capslock,	0b0100000000000000
		.set	S_sysreq,	0b1000000000000000
		.set	e0_bit,		0
		.set	e1_bit,		1
		.set	AltUp_bit,	2
		.set	e1CtrlBit,	3
		.set	S_e0,		0b00000000000000000000000000000001
		.set	S_e1,		0b00000000000000000000000000000010
		.set	S_e0e1,		0b00000000000000000000000000000011

		.text
		.align	4
k_start_of_text:
_k_enable:.globl _k_enable
		# enable the keyboard interface
		call	_k_wait
		movb	$0xae,%al
		outb	%al,$0x64
		ret

		.align	4
_k_disable:.globl _k_disable
		# disable the keyboard interface
		call	_k_wait
		movb	$0xad,%al
		outb	%al,$0x64
		call	_k_wait
		ret

		.align	4
		# wait for the keyboard command buffer to be clear
_k_wait:.globl _k_wait
		movl	$0x20000,%ecx
1:		jcxz	2f
2:		inb		$0x64,%al
		testb	$2,%al
		loopnz	1b
		ret

		.align	4

_k_getScan:.globl _k_getScan
		# get the scan code (or data byte) from the keyboard data buffer
		inb		$0x60,%al
		movzbl	%al,%eax
		ret

		.align	4
_k_putScan:.globl _k_putScan
		# write a scancode/data byte/command to the keyboard data/cmd buffer
		.byte	0x36
		movb	4(%esp),%al
		outb	%al,$0x60
		ret

		.align	4
_k_getStat:.globl _k_getStat
		# get the keyboard status
		inb		$0x64,%al
		movzbl	%al,%eax
		ret

		.align	4
_k_putStat:.globl _k_putStat
		# write data/command to the keyboard status register
		.byte	0x36
		movb	4(%esp),%al
		outb	%al,$0x64
		ret

		.align	4
k_wrapper:
		# wrapper function
		pushal
		pushl	%ds
		pushl	%es
		pushl	%fs
		pushl	%gs

		# setup %ds and %es to point to our data segment
		.byte	0x2e			# %cs prefix, for gas botch
		movw	k_data_segment,%ds
		movw	k_data_segment,%es

		call	k_isr

		popl	%gs
		popl	%fs
		popl	%es
		popl	%ds
		movb	$0x20,%al
		outb	%al,$0x20
		popal
		nop
		iret

		.align	4
k_isr:
		call	_k_disable
		call	_k_getScan
		movzbl	%al,%eax
		call	*scan_table(,%eax,4)
		call	_k_enable
		ret

		.align	2
escape_0:
		btsl	$e0_bit,status
		ret

		.align	2
escape_1:
		btsl	$e1_bit,status
		ret
#
# lock keys
#
		.align	2
caps_lock_u:
		andw	$~S_capslock,shifts
		jmp		isr_exit
		.align	2
caps_lock_d:
		testw	$S_capslock,shifts
		jnz		isr_exit
		orw		$S_capslock,shifts
		xorw	$S_CapsLock,shifts
		jmp		do_leds
		.align	2
num_lock_u:
		andw	$~S_numlock,shifts
		jmp		isr_exit
		.align	2
num_lock_d:
		btl		$e1CtrlBit,status
		jnc		2f
		movl	pause,%eax
		orl		%eax,%eax
		jz		1f
		call	*%eax
1:		jmp		isr_exit
2:		testw	$S_numlock,shifts
		jnz		isr_exit
		orw		$S_numlock,shifts
		xorw	$S_NumLock,shifts
		jmp		do_leds
		.align	2
scrl_lock_u:
		andw	$~S_scrllock,shifts
		jmp		isr_exit
		.align	2
scrl_lock_d:
		testw	$S_scrllock,shifts
		jnz		isr_exit
		orw		$S_scrllock,shifts
		xorw	$S_ScrlLock,shifts
		.align	2
do_leds:
2:		pushl	$0xed
		call	_k_putScan
		addl	$4,%esp
1:		call	_k_getStat
		testb	$1,%al
		jz		1b
		call	_k_getScan
		cmpb	$0xfa,%al
		jne		2b
2:		movb	shifts,%al
		andb	$Leds,%al
		shrb	$LedsShift,%al
		push	%eax
		call	_k_putScan
1:		call	_k_getStat
		testb	$1,%al
		jz		1b
		call	_k_getScan
		cmpb	$0xfa,%al
		jne		2b
		addl	$4,%esp
		ret
#
# standard keys
#
		.align	2
std_key:
		movb	%al,scan_code
		testw	$S_Alt,shifts
		jnz		3f
		testw	$S_Ctrl,shifts
		jnz		2f
		testw	$S_Shift,shifts
		jnz		1f
		movb	norm_keys(%eax),%al
		jmp		4f
1:
		movb	shift_keys(%eax),%al
		jmp		4f
2:
		movb	ctrl_keys(%eax),%al
		jmp		4f
3:
		movb	alt_keys(%eax),%al
		cmpb	$0xff,%al
		je		7f
		movb	%al,scan_code
		movb	$0,%al
4:
		cmpb	$0xff,%al
		je		7f
		cmpb	$'A',%al
		jb		6f
		cmpb	$'Z',%al
		jbe		5f
		cmpb	$'a',%al
		jb		6f
		cmpb	$'z',%al
		jbe		5f
		jmp		6f
5:
		testw	$S_CapsLock,shifts
		jz		6f
		xorb	$0x20,%al
6:
		movb	%al,char_code
		call	*put_key
7:		jmp		isr_exit
#
# funciton keys (F1-F12)
#
		.align	2
func_key:
		movb	$0,char_code
		testw	$S_Alt,shifts
		jnz		3f
		testw	$S_Ctrl,shifts
		jnz		2f
		testw	$S_Shift,shifts
		jnz		1f
		movb	norm_keys(%eax),%al
		jmp		4f
1:
		movb	shift_keys(%eax),%al
		jmp		4f
2:
		movb	ctrl_keys(%eax),%al
		jmp		4f
3:
		movb	alt_keys(%eax),%al
4:
		movb	%al,scan_code
		call	*put_key
		jmp		isr_exit
#
# shift keys
#
		.align	2
l_shift_u:
		btl		$e0_bit,status
		jc		1f
		andw	$~S_LShift,shifts
1:		jmp		isr_exit
		.align	2
l_shift_d:
		btl		$e0_bit,status
		jc		1f
		orw		$S_LShift,shifts
1:		jmp		isr_exit
		.align	2
r_shift_u:
		btl		$e0_bit,status
		jc		1f
		andw	$~S_RShift,shifts
1:		jmp		isr_exit
		.align	2
r_shift_d:
		btl		$e0_bit,status
		jc		1f
		orw		$S_RShift,shifts
1:		jmp		isr_exit
		.align	2
ctrl_u:
		btl		$e1_bit,status
		jnc		1f
		btrl	$e1CtrlBit,status
		jmp		isr_exit
1:		btl		$e0_bit,status
		jc		2f
		andw	$~S_LCtrl,shifts
		jmp		3f
2:		andw	$~S_RCtrl,shifts
3:		testw	$S_ctrl,shifts
		jnz		4f
		andw	$~S_Ctrl,shifts
4:		jmp		isr_exit
		.align	2
ctrl_d:
		btl		$e1_bit,status
		jnc		1f
		btsl	$e1CtrlBit,status
		jmp		isr_exit
1:		btl		$e0_bit,status
		jc		2f
		orw		$S_LCtrl|S_Ctrl,shifts
		jmp		3f
2:		orw		$S_RCtrl|S_Ctrl,shifts
3:		jmp		isr_exit
		.align	2
alt_u:
		btl		$e0_bit,status
		jc		1f
		andw	$~S_LAlt,shifts
		jmp		2f
1:		andw	$~S_RAlt,shifts
2:		testw	$S_alt,shifts
		jnz		3f
		andw	$~S_Alt,shifts
3:		movzbl	make_char,%eax
		orl		%eax,%eax
		jz		6f
4:		movw	%ax,char_code
		movb	$0,make_char
		call	*put_key
5:		jmp		isr_exit
6:		bt		$AltUp_bit,status
		jnc		5b
		xorw	%ax,%ax
		decw	%ax
		jmp		4b
		.align	2
alt_d:
		btl		$e0_bit,status
		jc		1f
		orw		$S_LAlt|S_Alt,shifts
		jmp		2f
1:		orw		$S_RAlt|S_Alt,shifts
2:		jmp		isr_exit
#
# keypad keys
#
		.align	2
pad_key:
		btl		$e0_bit,status
		jnc		4f
		# grey keys
		movb	$0,char_code
		testw	$S_Alt,shifts
		jnz		2f
		testw	$S_Ctrl,shifts
		jnz		1f
		movb	norm_keys(%eax),%al
		jmp		3f
1:		movb	ctrl_keys(%eax),%al
		jmp		3f
2:		cmpb	$0x53,%al
		jne		1f
		testw	$S_Alt|S_Ctrl,shifts
		jnz		rb
		
1:		movb	alt_keys(%eax),%al
3:		movb	%al,scan_code
		call	*put_key
		jmp		isr_exit
		# key pad keys
4:		testw	$S_Alt,shifts
		jnz		5f
		testw	$S_Ctrl,shifts
		jnz		4f
		testw	$S_Shift,shifts
		jnz		1f
		testw	$S_NumLock,shifts
		jnz		3f
		jmp		2f
1:		testw	$S_NumLock,shifts
		jz		3f
		# !Shift or Shift and NumLock
2:		movb	norm_keys(%eax),%ah
		xorb	%al,%al
		jmp		8f
		# Shift or NumLock
3:		movb	shift_keys(%eax),%ah
		xchgb	%al,%ah
		jmp		8f
		# Ctrl
4:		movb	ctrl_keys(%eax),%ah
		xorb	%al,%al
		jmp		8f
		# Alt
5:		cmpb	$0x53,%al
		je		6f
		movzbl	make_char,%ebx
		leal	(%ebx,%ebx,4),%ebx
		shll	$1,%ebx
		movb    alt_keys(%eax),%al
		addb	%al,%bl
		movb	%bl,make_char
		jmp		isr_exit
6:		movw	shifts,%bx
		andw	$S_Alt|S_Ctrl,%bx
		cmpw	$S_Alt|S_Ctrl,%bx
		jnz		7f
rb:		movl	cad,%eax
		orl		%eax,%eax
		jz		isr_exit
		call	*%eax
		jmp		isr_exit
7:		movb	alt_keys(%eax),%ah
		xorb	%al,%al
8:		movw	%ax,char_code
		call	*put_key
		jmp		isr_exit
#
# enter key
#
		.align	2
enter_key:
		# needs work for gray key (how to represent?)
		testw	$S_Alt,shifts
		jnz		2f
		testw	$S_Ctrl,shifts
		jnz     1f
		movb	norm_keys(%eax),%ah
		xchgb	%al,%ah
		jmp		3f
1:		movb	ctrl_keys(%eax),%ah
		xchgb	%al,%ah
		jmp		3f
2:		movb	alt_keys(%eax),%ah
		xorb	%al,%al
3:		movw	%ax,char_code
		call	*put_key
		jmp		isr_exit
#
# tab key
#
		.align	2
tab_key:
		testw	$S_Alt,shifts
		jnz		3f
		testw	$S_Ctrl,shifts
		jnz     2f
		testw	$S_Shift,shifts
		jnz		1f
		movb	norm_keys(%eax),%ah
		xchgb	%al,%ah
		jmp		5f
1:		movb	%al,%ah
		jmp		4f
2:		movb	ctrl_keys(%eax),%ah
		jmp		4f
3:		movb	alt_keys(%eax),%ah
4:		xorb	%al,%al
5:		movw	%ax,char_code
		call	*put_key
		jmp		isr_exit
#
# common exit code
#
		.align	2
no_op:
isr_exit:
		andl	$~S_e0e1,status
		ret

		.align	2
_put_key:
		movzbl	head,%eax
		movl	_key_data,%ebx
		movl	%ebx,buffer(,%eax,4)
		incb	%al
		andb	$0x3f,%al
		cmpb	tail,%al
		je		1f
		movb	%al,head
		ret
1:		in		$0x61,%al
		xorb	$2,%al
		outb	%al,$0x61
		ret

		.align	4
k_end_of_text:
_k_getKey:.globl _k_getKey
		pushfl
		cli
		movb	tail,%al
		cmpb	%al,head
		je		1f
		movb	%al,%ah
		incb	%ah
		andb	$0x3f,%ah
		movb	%ah,tail
		movzbl	%al,%eax
		movl	buffer(,%eax,4),%eax
		popfl
		ret
1:		xor		%eax,%eax
		popfl
		ret

		.align	4
_k_init:.globl _k_init
		push	%ebp
		movl	%esp,%ebp
		pushl	%ebx
		pushl	%esi
        pushl	%edi

		movw	$0x10,%bx
		movw	%bx,k_data_segment

		# save old interrupt vector
		# cx:eax address (sel:offs)
		movb	$1,%al
		call	get_irq_vector
		movl	%eax,old_offs
		movw	%dx,old_sel

		# hook interrupt
		# cx:edx address (sel:offs)
		movw	%cs,%cx
		movl	$k_wrapper,%edx
		movb	$1,%al
		call	set_irq_vector

		# enable irq 1
		inb		$0x21,%al
		andb	$0xfd,%al
		outb	%al,$0x21

		xor		%eax,%eax
		popl	%edi
		popl	%esi
		popl	%ebx
		leave
		ret

		.align	2
_k_set_putKey:.globl _k_set_putKey
		movl	4(%esp),%eax
		xchgl	%eax,put_key
		ret

		.align	2
_k_set_CtrlAltDel:.globl _k_set_CtrlAltDel
		movl	4(%esp),%eax
		xchgl	%eax,cad
		ret

		.align	2
_k_set_pause:.globl _k_set_pause
		movl	4(%esp),%eax
		xchgl	%eax,pause
		ret

		.data
k_start_of_data:
scan_table:
# make codes
		.long	no_op				# 00-00 ** Error
		.long	std_key				# 01-01 Esc
		.long	std_key				# 02-02 1!
		.long	std_key				# 03-03 2@
		.long	std_key				# 04-04 3#
		.long	std_key				# 05-05 4$
		.long	std_key				# 06-06 5%
		.long	std_key				# 07-07 6^
		.long	std_key				# 08-08 7&
		.long	std_key				# 09-09 8*
		.long	std_key				# 0A-0A 9(
		.long	std_key				# 0B-0B 0)
		.long	std_key				# 0C-0C -_
		.long	std_key				# 0D-0D =+
		.long	std_key				# 0E-0E Backspace
		.long	tab_key				# 0F-0F Tab
		.long	std_key				# 10-10 qQ
		.long	std_key				# 11-11 wW
		.long	std_key				# 12-12 eE
		.long	std_key				# 13-13 rR
		.long	std_key				# 14-14 tT
		.long	std_key				# 15-15 yY
		.long	std_key				# 16-16 uU
		.long	std_key				# 17-17 iI
		.long	std_key				# 18-18 oO
		.long	std_key				# 19-19 pP
		.long	std_key				# 1A-1A [{
		.long	std_key				# 1B-1B ]}
		.long	enter_key			# 1C-1C Enter
		.long	ctrl_d				# 1D-1D Ctrl
		.long	std_key				# 1E-1E aA
		.long	std_key				# 1F-1F sS
		.long	std_key				# 20-20 dD
		.long	std_key				# 21-21 fF
		.long	std_key				# 22-22 gG
		.long	std_key				# 23-23 hH
		.long	std_key				# 24-24 jJ
		.long	std_key				# 25-25 kK
		.long	std_key				# 26-26 lL
		.long	std_key				# 27-27 ;:
		.long	std_key				# 28-28 '"
		.long	std_key				# 29-29 `~
		.long	l_shift_d			# 2A-2A Left Shift
		.long	std_key				# 2B-2B \|
		.long	std_key				# 2C-2C zZ
		.long	std_key				# 2D-2D xX
		.long	std_key				# 2E-2E cC
		.long	std_key				# 2F-2F vV
		.long	std_key				# 30-30 bB
		.long	std_key				# 31-31 nN
		.long	std_key				# 32-32 mM
		.long	std_key				# 33-33 ,<
		.long	std_key				# 34-34 .>
		.long	std_key				# 35-35 /?
		.long	r_shift_d			# 36-36 Right Shift
		.long	std_key				# 37-37 KP *
		.long	alt_d				# 38-38 Alt
		.long	std_key				# 39-39 Space
		.long	caps_lock_d			# 3A-3A Caps Lock
		.long	func_key			# 3B-3B F1
		.long	func_key			# 3C-3C F2
		.long	func_key			# 3D-3D F3
		.long	func_key			# 3E-3E F4
		.long	func_key			# 3F-3F F5
		.long	func_key			# 40-40 F6
		.long	func_key			# 41-41 F7
		.long	func_key			# 42-42 F8
		.long	func_key			# 43-43 F9
		.long	func_key			# 44-44 F10
		.long	num_lock_d			# 45-45 Num Lock
		.long	scrl_lock_d			# 46-46 Scroll Lock
		.long	pad_key				# 47-47 KP 7 (Home)
		.long	pad_key				# 48-48 KP 8 (Up Arrow)
		.long	pad_key				# 49-49 KP 9 (Page Up)
		.long	std_key				# 4A-4A KP -
		.long	pad_key				# 4B-4B KP 4 (Left Arrow)
		.long	pad_key				# 4C-4C KP 5
		.long	pad_key				# 4D-4D KP 6 (Right Arrow)
		.long	std_key				# 4E-4E KP +
		.long	pad_key				# 4F-4F KP 1 (End)
		.long	pad_key				# 50-50 KP 2 (Down Arrow)
		.long	pad_key				# 51-51 KP 3 (Page Down)
		.long	pad_key				# 52-52 KP 0 (Ins)
		.long	pad_key				# 53-53 KP . (Del)
		.long	no_op				# 54-54 Sys Req
		.long	no_op				# 55-55
		.long	no_op				# 56-56
		.long	func_key			# 57-57 F11
		.long	func_key			# 58-58 F12
		.long	no_op				# 59-59
		.long	no_op				# 5A-5A
		.long	no_op				# 5B-5B
		.long	no_op				# 5C-5C
		.long	no_op				# 5D-5D
		.long	no_op				# 5E-5E
		.long	no_op				# 5F-5F
		.long	no_op				# 60-60
		.long	no_op				# 61-61
		.long	no_op				# 62-62
		.long	no_op				# 63-63
		.long	no_op				# 64-64
		.long	no_op				# 65-65
		.long	no_op				# 66-66
		.long	no_op				# 67-67
		.long	no_op				# 68-68
		.long	no_op				# 69-69
		.long	no_op				# 6A-6A
		.long	no_op				# 6B-6B
		.long	no_op				# 6C-6C
		.long	no_op				# 6D-6D
		.long	no_op				# 6E-6E
		.long	no_op				# 6F-6F
		.long	no_op				# 70-70
		.long	no_op				# 71-71
		.long	no_op				# 72-72
		.long	no_op				# 73-73
		.long	no_op				# 74-74
		.long	no_op				# 75-75
		.long	no_op				# 76-76
		.long	no_op				# 77-77
		.long	no_op				# 78-78
		.long	no_op				# 79-79
		.long	no_op				# 7A-7A
		.long	no_op				# 7B-7B
		.long	no_op				# 7C-7C
		.long	no_op				# 7D-7D
		.long	no_op				# 7E-7E
		.long	no_op				# 7F-7F
# break codes
		.long	no_op				# 80-00
		.long	no_op				# 81-01 Esc
		.long	no_op				# 82-02 1!
		.long	no_op				# 83-03 2@
		.long	no_op				# 84-04 3#
		.long	no_op				# 85-05 4$
		.long	no_op				# 86-06 5%
		.long	no_op				# 87-07 6^
		.long	no_op				# 88-08 7&
		.long	no_op				# 89-09 8*
		.long	no_op				# 8A-0A 9(
		.long	no_op				# 8B-0B 0)
		.long	no_op				# 8C-0C -_
		.long	no_op				# 8D-0D =+
		.long	no_op				# 8E-0E Backspace
		.long	no_op				# 8F-0F Tab
		.long	no_op				# 90-10 qQ
		.long	no_op				# 91-11 wW
		.long	no_op				# 92-12 eE
		.long	no_op				# 93-13 rR
		.long	no_op				# 94-14 tT
		.long	no_op				# 95-15 yY
		.long	no_op				# 96-16 uU
		.long	no_op				# 97-17 iI
		.long	no_op				# 98-18 oO
		.long	no_op				# 99-19 pP
		.long	no_op				# 9A-1A [{
		.long	no_op				# 9B-1B ]}
		.long	no_op				# 9C-1C Enter
		.long	ctrl_u				# 9D-1D Ctrl
		.long	no_op				# 9E-1E aA
		.long	no_op				# 9F-1F sS
		.long	no_op				# A0-20 dD
		.long	no_op				# A1-21 fF
		.long	no_op				# A2-22 gG
		.long	no_op				# A3-23 hH
		.long	no_op				# A4-24 jJ
		.long	no_op				# A5-25 kK
		.long	no_op				# A6-26 lL
		.long	no_op				# A7-27 ;:
		.long	no_op				# A8-28 '"
		.long	no_op				# A9-29 `~
		.long	l_shift_u			# AA-2A Left Shift
		.long	no_op				# AB-2B \|
		.long	no_op				# AC-2C zZ
		.long	no_op				# AD-2D xX
		.long	no_op				# AE-2E cC
		.long	no_op				# AF-2F vV
		.long	no_op				# B0-30 bB
		.long	no_op				# B1-31 nN
		.long	no_op				# B2-32 mM
		.long	no_op				# B3-33 ,<
		.long	no_op				# B4-34 .>
		.long	no_op				# B5-35 /?
		.long	r_shift_u			# B6-36 Right Shift
		.long	no_op				# B7-37 KP
		.long	alt_u				# B8-38 Alt
		.long	no_op				# B9-39 Space
		.long	caps_lock_u			# BA-3A Caps Lock
		.long	no_op				# BB-3B F1
		.long	no_op				# BC-3C F2
		.long	no_op				# BD-3D F3
		.long	no_op				# BE-3E F4
		.long	no_op				# BF-3F F5
		.long	no_op				# C0-40 F6
		.long	no_op				# C1-41 F7
		.long	no_op				# C2-42 F8
		.long	no_op				# C3-43 F9
		.long	no_op				# C4-44 F10
		.long	num_lock_u			# C5-45 Num Lock
		.long	scrl_lock_u			# C6-46 Scroll Lock
		.long	no_op				# C7-47 KP 7 (Home)
		.long	no_op				# C8-48 KP 8 (Up Arrow)
		.long	no_op				# C9-49 KP 9 (Page Up)
		.long	no_op				# CA-4A KP -
		.long	no_op				# CB-4B KP 4 (Left Arrow)
		.long	no_op				# CC-4C KP 5
		.long	no_op				# CD-4D KP 6 (Right Arrow)
		.long	no_op				# CE-4E KP +
		.long	no_op				# CF-4F KP 1 (End)
		.long	no_op				# D0-50 KP 2 (Down Arrow)
		.long	no_op				# D1-51 KP 3 (Page Down)
		.long	no_op				# D2-52 KP 0 (Ins)
		.long	no_op				# D3-53 KP . (Del)
		.long	no_op				# D4-54 Sys Req
		.long	no_op				# D5-55
		.long	no_op				# D6-56
		.long	no_op				# D7-57 F11
		.long	no_op				# D8-58 F12
		.long	no_op				# D9-59
		.long	no_op				# DA-5A
		.long	no_op				# DB-5B
		.long	no_op				# DC-5C
		.long	no_op				# DD-5D
		.long	no_op				# DE-5E
		.long	no_op				# DF-5F
		.long	escape_0			# E0-60 ** Escape 0
		.long	escape_1			# E1-61 ** Escape 1
		.long	no_op				# E2-62
		.long	no_op				# E3-63
		.long	no_op				# E4-64
		.long	no_op				# E5-65
		.long	no_op				# E6-66
		.long	no_op				# E7-67
		.long	no_op				# E8-68
		.long	no_op				# E9-69
		.long	no_op				# EA-6A
		.long	no_op				# EB-6B
		.long	no_op				# EC-6C
		.long	no_op				# ED-6D
		.long	no_op				# EE-6E
		.long	no_op				# EF-6F
		.long	no_op				# F0-70
		.long	no_op				# F1-71
		.long	no_op				# F2-72
		.long	no_op				# F3-73
		.long	no_op				# F4-74
		.long	no_op				# F5-75
		.long	no_op				# F6-76
		.long	no_op				# F7-77
		.long	no_op				# F8-78
		.long	no_op				# F9-79
		.long	no_op				# FA-7A ** Acknowledge
		.long	no_op				# FB-7B
		.long	no_op				# FC-7C ** BAT Failure code
		.long	no_op				# FD-7D
		.long	no_op				# FE-7E ** Resend (Nack)
		.long	no_op				# FF-7F **

alt_keys:
		.byte	0x00
		.byte	0x01,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E		# 01-08
		.byte	0x7F,0x80,0x81,0x82,0x83,0x0E,0xA5,0x10		# 09-10
		.byte	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18		# 11-18
		.byte	0x19,0x1A,0x1B,0xA6,0xFF,0x1E,0x1F,0x20		# 19-20
		.byte	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28		# 21-28
		.byte	0x29,0xFF,0x26,0x2C,0x2D,0x2E,0x2F,0x30		# 29-30
		.byte	0x31,0x32,0x33,0x34,0x35,0xFF,0x37,0xFF		# 31-38
		.byte	0x39,0xFF,0x68,0x69,0x6A,0x6B,0x6C,0x6D		# 39-40
		.byte	0x6E,0x6F,0x70,0x71,0xFF,0xFF,0x07,0x08		# 41-48
		.byte	0x09,0x4A,0x04,0x05,0x06,0x4E,0x01,0x02		# 49-50
		.byte	0x03,0x00,0xA3,0xFF,0xFF,0xFF,0x8B,0x8C		# 51-58
		.byte	0xBB,0xE2,0xB3,0xE2,0x81,0x79,0x31,0x69		# 59-60
		.byte	0xB1,0xC1,0xB9,0x59,0xFB,0xF2,0xE1,0xE1		# 61-68
		.byte	0xD1,0x61,0x61,0xFB,0x92,0xD1,0x59,0xB9		# 69-70
		.byte	0x81,0xD9,0x61,0xFB,0xB2,0x89,0xE1,0x8B		# 71-78
		.byte	0xFF,0xFE,0x67,0x04,0x03,0xFF,0x00			# 79-7f

ctrl_keys:
		.byte	0x00
		.byte	0x1B,0xFF,0x00,0xFF,0xFF,0xFF,0x1E,0xFF		# 01-08
		.byte	0xFF,0xFF,0xFF,0x1F,0xFF,0x7F,0x94,0x11		# 09-10
		.byte	0x17,0x05,0x12,0x14,0x19,0x15,0x09,0x0F		# 11-18
		.byte	0x10,0x1B,0x1D,0x0A,0xFF,0x01,0x13,0x04		# 19-20
		.byte	0x06,0x07,0x08,0x0A,0x0B,0x0C,0xFF,0xFF		# 21-28
		.byte	0xFF,0xFF,0x1C,0x1A,0x18,0x03,0x16,0x02		# 29-30
		.byte	0x0E,0x0D,0xFF,0xFF,0x95,0xFF,0x96,0xFF		# 31-38
		.byte	0x20,0xFF,0x5E,0x5F,0x60,0x61,0x62,0x63		# 39-40
		.byte	0x64,0x65,0x66,0x67,0x45,0x46,0x77,0x8D		# 41-48
		.byte	0x84,0x8E,0x73,0x8F,0x74,0x90,0x75,0x91		# 49-50
		.byte	0x76,0x92,0x93,0xFF,0xFF,0xFF,0x89,0x8A		# 51-58
		.byte	0x5F,0xF2,0x5B,0x7F,0xCA,0xCA,0xF2,0x7F		# 59-60
		.byte	0xEE,0x5A,0x62,0x5A,0x2E,0x7A,0x4E,0x7F		# 61-68
		.byte	0xB2,0x1A,0x32,0x2E,0x6A,0x4A,0x32,0x4F		# 69-70
		.byte	0x7F,0xDA,0xC6,0xEE,0xDA,0xFA,0x4F,0x7F		# 71-78
		.byte	0x3B,0x3B,0x4B,0x3B,0x33,0x4B,0x1F			# 79-7f

shift_keys:
		.byte	0x00
		.byte	0x1B,0x21,0x40,0x23,0x24,0x25,0x5E,0x26		# 01-08
		.byte	0x2A,0x28,0x29,0x5F,0x2B,0x08,0x00,0x51		# 09-10
		.byte	0x57,0x45,0x52,0x54,0x59,0x55,0x49,0x4F		# 11-18
		.byte	0x50,0x7B,0x7D,0x0D,0xFF,0x41,0x53,0x44		# 19-20
		.byte	0x46,0x47,0x48,0x4A,0x4B,0x4C,0x3A,0x22		# 21-28
		.byte	0x7E,0xFF,0x7C,0x5A,0x58,0x43,0x56,0x42		# 29-30
		.byte	0x4E,0x4D,0x3C,0x3E,0x3F,0xFF,0x2A,0xFF		# 31-38
		.byte	0x20,0xFF,0x54,0x55,0x56,0x57,0x58,0x59		# 39-40
		.byte	0x5A,0x5B,0x5C,0x5D,0xFF,0xFF,0x37,0x38		# 41-48
		.byte	0x39,0x2D,0x34,0x35,0x36,0x2B,0x31,0x32		# 49-50
		.byte	0x33,0x30,0x2E,0xFF,0xFF,0x7C,0x87,0x88		# 51-58
		.byte	0x2A,0x35,0x36,0x37,0x38,0x39,0x3F,0x2F		# 59-60
		.byte	0xBB,0xE2,0xB3,0xE2,0x81,0x79,0x31,0x69		# 61-68
		.byte	0xB1,0xC1,0xB9,0x59,0xFB,0xF2,0xE1,0xE1		# 69-70
		.byte	0xD1,0x61,0x61,0xFB,0x92,0xD1,0x59,0xB9		# 71-78
		.byte	0x81,0xD9,0x61,0xFB,0xB2,0x89,0xE1			# 79-7f

norm_keys:
		.byte	0x00
		.byte	0x1B,0x31,0x32,0x33,0x34,0x35,0x36,0x37		# 01-08
		.byte	0x38,0x39,0x30,0x2D,0x3D,0x08,0x09,0x71		# 09-10
		.byte	0x77,0x65,0x72,0x74,0x79,0x75,0x69,0x6F		# 11-18
		.byte	0x70,0x5B,0x5D,0x0D,0xFF,0x61,0x73,0x64		# 19-20
		.byte	0x66,0x67,0x68,0x6A,0x6B,0x6C,0x3B,0x27		# 21-28
		.byte	0x60,0xFF,0x5C,0x7A,0x78,0x63,0x76,0x62		# 29-30
		.byte	0x6E,0x6D,0x2C,0x2E,0x2F,0xFF,0x2A,0xFF		# 31-38
		.byte	0x20,0xFF,0x3B,0x3C,0x3D,0x3E,0x3F,0x40		# 39-40
		.byte	0x41,0x42,0x43,0x44,0xFF,0xFF,0x47,0x48		# 41-48
		.byte	0x49,0x2D,0x4B,0xF0,0x4D,0x2B,0x4F,0x50		# 49-50
		.byte	0x51,0x52,0x53,0xFF,0xFF,0x5C,0x85,0x86		# 51-58
		.byte	0x5F,0xF2,0x5B,0x7F,0xCA,0xCA,0xF2,0x7F		# 59-60
		.byte	0xEE,0x5A,0x62,0x5A,0x2E,0x7A,0x4E,0x7F		# 61-68
		.byte	0xB2,0x1A,0x32,0x2E,0x6A,0x4A,0x32,0x4F		# 69-70
		.byte	0x7F,0xDA,0xC6,0xEE,0xDA,0xFA,0x4F,0x7F		# 71-78
		.byte	0x3B,0x3B,0x4B,0x3B,0x33,0x4B,0x1F			# 79-7f

		.align	4
put_key:
		.long	_put_key
cad:
		.long	0
pause:
		.long	0
_key_data:.global _key_data
char_code:
		.byte	0
scan_code:
		.byte	0
shifts:
		.short	0
status:
		.long	0
old_offs:
		.long	0
old_sel:
		.short	0
k_data_segment:
		.short	0
make_char:
		.byte	0
head:
		.byte	0
tail:
		.byte	0
		.align	4
buffer:
		.fill	64,4,0
k_end_of_data:

		.section	.ctor
		.long		_k_init
