short dispatch(short ax)
{
	switch (ax) {
	/* LDT Management services */
	case 0x0000:/* 0.9 Allocate LDT Descriptor*/
		/* Allocates one or more descriptors in the task's Descriptor Table 
		 * (LDT).  The descriptor(s) allocated must be initialized by the
		 * application with other function calls.
		 *
		 * Call With:
		 *		AX			= 0x0000
		 *		CX			= number of descriptors to allocate
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *		AX			= base selector
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8011	descriptor unavailable
		 *
		 * Notes:
		 *
		 * o	If more that one descriptor was requested, the function
		 *		returns a base selector referencing the first of a contiguous
		 *		array of descriptors.  The selector walues for subsequent
		 *		descriptors in the array can be calculated by adding the value
		 *		returned by Int 31H Function 0003H
		 *
		 * o	The allocated descriptor(s) will be set to "data" with the
		 *		present bit set and a base and limit of zero.  The privilge
		 *		level of the descriptor(s) will match the appication's code
		 *		segment privilege level.
		 */
	case 0x0001:/* 0.9 Free LDT Descriptor*/
		/* Frees an LDT descriptor.
		 *
		 * Call With:
		 *		AX			= 0x0001
		 *		BX			= selector for the descriptor to free
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8022	invalid selector
		 *
		 * Notes:
		 *
		 * o	Each descriptor allocated with Int 31H Function 0000H must be
		 *		freed individually with this function, even if it was
		 *		previously allocated as part of a contiguous array of
		 *		descriptors.
		 *
		 * o	Under DPMI 1.0 host, any segment registers which contain the
		 *		selector being freed are zeroed by this function.
		 */
	case 0x0002:/* 0.9 Map Real-Mode Segment to Descriptor*/
		/* Maps a real mode segment (paragraph) address onto an LDT descriptor
		 * that can be used by a protected mode program to access the same
		 * memory.
		 *
		 * Call With:
		 *		AX			= 0x0002
		 *		BX			= real mode segment address
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *		AX			= selector for real mode segment
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8011	descriptor unavailable
		 *
		 * Notes:
		 *
		 * o	The descriptor's limit will be set to 64 KB.
		 *
		 * o	Multiple calls to this function with the same segment address
		 *		will return the same selector.
		 *
		 * o	The intent of this function is to provide clients with easy
		 *		access to commonly used real mode segments such as the BIOS
		 *		data area at segment 0040H and the video refresh buffers at
		 *		segments A000H, B000H, and B800H.  Clients should not call
		 *		this function to obtain descriptors to private data areas.
		 *
		 * o	Descriptors created by this function can never be modified or
		 *		freed.  For this reason, the function should be used
		 *		sparingly.  Clients which need to examine various real mode
		 *		addresses with the same selector should allocate a descriptor
		 *		with Int 31H Function 0000H and change the base address int
		 *		the descriptor as necessary, using the Set Segment Base
		 *		Address function (Int 31H Function 0007H).
		 */
	case 0x0003:/* 0.9 Get Selector Increment Value*/
		/* The DPMI functions Allocate LDT Descriptors (Int 31H Function
		 * 0000H) and Allocate DOS Memory Block (Int 31H Function 0100H) can
		 * allocate an array of contiguous descriptors, but only return a
		 * selector for the first descriptor.  The value returned by this
		 * function can be used to calculate the selectors for subsequent
		 * descriptors on the array.
		 *
		 * Call With:
		 *		AX			= 0x0003
		 *
		 * Returns:
		 *
		 *		Carry flag	= clear
		 *		AX			= selector increment value
		 *
		 * Notes:
		 *
		 * o	The increment value is always a power of two.
		 */
	case 0x0004:/*reserved*/
		/* DPMI Function 0004H is reserved for historical reasons and should
		 * not be called.
		 *
		 * Lock selector
		 *
		 * undocumented (from Ralf Brown's interrupt list)
--------E-310004-----------------------------
INT 31 P - DPMI 0.9+ - LOCK SELECTOR
		AX = 0004h
		BX = selector to lock (prevent paging)
Return: unknown
Note:	although marked as reserved in versions 0.9 and 1.0 of the DPMI
		  specification, this function is called by MS Windows TASKMAN,
			  PROGMAN, and KERNEL
		 *
		 * Call With:
		 *		AX			= 0x0004
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0005:/*reserved*/
		/* DPMI Function 0005H is reserved for historical reasons and should
		 * not be called.
		 *
		 * undocumented (from Ralf Brown's interrupt list)
--------E-310005-----------------------------
INT 31 P - DPMI 0.9+ - UNLOCK SELECTOR
		AX = 0005h
		BX = selector to unlock (permit paging)
Return: unknown
Note:	although marked as reserved in versions 0.9 and 1.0 of the DPMI
		  specification, this function is called by MS Windows TASKMAN,
		  PROGMAN, and KERNEL
		 *
		 * Call With:
		 *		AX			= 0x0005
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0006:/* 0.9 Get Segment Base Address*/
		/* Returns the 32-bit linear base address from the LDT descriptor for
		 * the specified segment.
		 *
		 * Call With:
		 *		AX			= 0x0006
		 *		BX			= selector
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *		CX:DX		= 32-bit linear base address of segment
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8022	invalid selector
		 *
		 * Notes:
		 *
		 * o	Client programs must use the LSL instruction to query the
		 *		limit for a descriptor.  Not that on 80386 machines, the
		 *		client must use the 32-bit form of LSL if the sement size is
		 *		greater that 64 KB.
		 */
	case 0x0007:/* 0.9 Set Segment Base Address*/
		/* Set the 32-bit linear base address field in the LDT descriptor for
		 * the specified segment.
		 *
		 * Call With:
		 *		AX			= 0x0007
		 *		BX			= selector
		 *		CX:DX		= 32-bit linear base address of segment
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8022	invalid selector
		 *						0x8025	invalid linear address (changing the
		 *								base would cause the descriptor to
		 *								reference a linear address range
		 *								outside that allowed for DPMI clients)
		 *
		 * Notes:
		 *
		 * o	A DPMI 1.0 host will automatically reload any segment register
		 *		which contains the selector specified in register BX.  It is
		 *		sugested that DPMI 0.9 hosts also implement this.
		 */
	case 0x0008:/* 0.9 Set Segment Limit*/
		/* Sets the limit field in the LDT descriptor for the specified
		 * segment.
		 *
		 * Call With:
		 *		AX			= 0x0008
		 *		BX			= selector
		 *		CX:DX		= 32-bit segment limit
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8021	invalid value (CX <> 0 on a 16-bit
		 *								DPMI host; or the limit is greater
		 *								that 1 MB, but the low twelve bits are
		 *								not set)
		 *						0x8022	invalid selector
		 *						0x8025	invalid linear address (changing the
		 *								limit would cause the descriptor to
		 *								reference a linear address range
		 *								outside that allowed for DPMI clients)
		 *
		 * Notes:
		 *
		 * o	The value supplied to the function is CX:DX is the byte length
		 *		of the segment-1 (i.e., the valued returned byt the LSL
		 *		instruction).
		 *
		 * o	Segment limits greater that or equal to 1 MB must be page-
		 *		aligned.  That is, limits greater that 1 MB must have the low
		 *		12 bits set.
		 *
		 * o	This function has an implicit effect on the "G" (granularity)
		 *		bit in an 80386 descriptor's exteded access rights/type byte;
		 *		i.e., it is the host's responsibility to set the "G" bit
		 *		correctly.
		 *
		 * o	A DPMI 1.0 host will automatically reload any segment register
		 *		which contains the selector specified in register BX.  It is
		 *		sugested that DPMI 0.9 hosts also implement this.
		 */
	case 0x0009:/* 0.9 Set Descriptor Access Rights*/
		/* Modifies the access rights ond type fields in the LDT descriptor
		 * for the specified segment.
		 *
		 * Call With:
		 *		AX			= 0x0009
		 *		BX			= selector
		 *		CL			= access rights/type byte
		 *		CH			= 80386 extended access rights/type byte
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8021	invalid value (access rights/type
		 *								bytes invalid)
		 *						0x8022	invalid selector
		 *						0x8025	invalid linear address (changing the
		 *								access rights/type would cause the
		 *								descriptor to reference a linear
		 *								address range outside that allowed for
		 *								DPMI clients)
		 *
		 * Notes:
		 *
		 * o	The access rights/type byte passed to the function in CL has
		 *		the following format:
		 *
		 *		+---+---+---+---+---+---+---+---+
		 *		| P |  DPL  | 1 |C/D|E/C|W/R| A |
		 *		+---+---+---+---+---+---+---+---+
		 *		  | 	|	  |   |   |   |   |
		 *		  | 	|	  |   |   |   |   +- 0=not accessed, 1=accessed
		 *		  | 	|	  |   |   |   +----- data: 0=read, 1=>read/write
		 *		  | 	|	  |   |   | 		 code: must be 1 (readable)
		 *		  | 	|	  |   |   +------ data: 0=expand-up, 1=expand down
		 *		  | 	|	  |   | 		  code: must be 0 (non-conforming)
		 *		  | 	|	  |   +------------- 0=data, 1=code
		 *		  | 	|	  +----------------- must be 1
		 *		  | 	+----------------------- must equal caller's CPL
		 *		  +----------------------------- 0=absent, 1=present
		 *
		 *		If the Present bit is not set in the descriptor, the DPMI host
		 *		allows any values except in the DPL and "must be 1" bit
		 *		fields.
		 *
		 * o	On 80386 (and later) machines, the DPMI host interprets the
		 *		value passed to the function in CH as follows:
		 *
		 *		+---+---+---+---+---+---+---+---+
		 *		| G |B/D| 0 |Avl|    Reserved   |
		 *		+---+---+---+---+---+---+---+---+
		 *		  |   |   |   | 		|
		 *		  |   |   |   | 		+-- ignored
		 *		  |   |   |   +------------ can be 0 or 1
		 *		  |   |   +---------------- must be 0
		 *		  |   +-------------------- 0=default 16-bit, 1=default 32-bit
		 *		  +------------------------ 0=byte granular, 1=page granular
		 *
		 * o	A DPMI 1.0 host will reload any segment registers which
		 *		contain the selector specified in register BX. It is suggested
		 *		that DPMI 0.9 hosts also implement this.
		 *
		 * o	Client programs should use the LAR instruction to examine the
		 *		access rights of a descriptor.
		 */
	case 0x000a:/* 0.9 Create Alias Descriptor*/
		/* Creates a new LDT data descriptor that has the same base and limit
		 * as the specified descriptor.
		 *
		 * Call With:
		 *		AX			= 0x000a
		 *		BX			= selector
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *		AX			= data selector (alias)
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8011	descriptor unavailable
		 *						0x8022	invalid selector
		 *
		 * Notes:
		 *
		 * o	The selector supplied to the function may be either a data
		 *		selector or an executable selector.  Note that the published
		 *		0.9 specification was in error to say that the function
		 *		generates an error on a data descriptor.
		 *
		 * o	The descriptor alias returned by this function will not track
		 *		changes to the original descriptor.  In other words, if an
		 *		alias is created with this function, and the base or limit of
		 *		the original segment is then changed, the two descriptors will
		 *		no longer map the same memory.
		 */
	case 0x000b:/* 0.9 Get Descriptor*/
		/* Copies the local descriptor table (LDT) entry for the specified
		 * selector into an 8-byte buffer.
		 *
		 * Call With:
		 *		AX			= 0x000b
		 *		BX			= selector
		 *		ES:(E)DI	= selector:offset of 8-byte buffer
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *		and buffer pointer to by ES:(E)DI contains descriptor
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8022	invalid selector
		 *
		 * Notes:
		 *
		 * o	32-bit programs must use ES:EDI to point to the buffer. 16-bit
		 *		programs should use ES:DI.
		 */
	case 0x000c:/* 0.9 Set Descriptor*/
		/* Copies the contents of an 8-byte buffer into the LDT descriptor for
		 * the specified selecotr.
		 *
		 * Call With:
		 *		AX			= 0x000c
		 *		BX			= selector
		 *		ES:(E)DI	= selector:offset of 8-byte buffer
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8021	invalid value (access rights/type
		 *								invalid)
		 *						0x8022	invalid selector
		 *						0x8025	invalid linear address (descriptor
		 *								references a linear address range
		 *								outside that allowed for DPMI clients)
		 *
		 * Notes:
		 *
		 * o	32-bit programs must use ES:EDI to point to the buffer. 16-bit
		 *		programs should use ES:DI.
		 *
		 * o	The descriptor's access rights/type bytes (byte 5) follows the
		 *		same format and restrictions as the access rights/type
		 *		parameter (in CL) for the Set Descriptor Access Rights
		 *		function (int 31H Function 0009H).  On 80386 (or later)
		 *		machines, the descriptor's extended access rights/type byte
		 *		(byte 6) follows the same format and restrictions as the
		 *		extended axxess rights/type parameter (in CH) for the same
		 *		function, except the low-order 4 vits (marked "reserved") are
		 *		used to set the upper 4 bits of the descriptor's limit.
		 *
		 * o	If the descriptor's present bit is not set, then the only
		 *		error checking is that the client's CPL must be equal to the
		 *		descriptor's DPL field and the "must be 1" bit in the
		 *		descriptor's byte 5 must be set.
		 *
		 * o	A DPMI 1.0 host will reload any segment registers which
		 *		contain the selector specified in register BX. It is suggested
		 *		that DPMI 0.9 hosts also implement this.
		 */
	case 0x000d:/* 0.9 Allocate Specific LDT Descriptor*/
		/* Allocates a specific LDT descriptor.
		 * Call With:
		 *		AX			= 0x000d
		 *		BX			= selector
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *		and descriptor has been allocated
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8011	descriptor unavailable (descriptor is
		 *								in use)
		 *						0x8022	invalid selector (references GDT or
		 *								beyond the LDT limit)
		 *
		 * Notes:
		 *
		 * o	The first 10H (16) descriptors (selector values 04H-7CH) are
		 *		reserved for this function and must not be used by the DPMI
		 *		host.
		 *
		 * o	Under DPMI 0.9 hosts, if another application has already been
		 *		loaded, some of descriptors reserved for allocation by this
		 *		function may be already in use and unavailable.  Under DPMI
		 *		1.0 hosts, each client has its own LDT and thus will have the
		 *		full 16 descriptors available for use with this function.
		 *
		 * o	Resident service providers (protected-mode TSRs) should NOT
		 *		use this function.
		 */
	case 0x000e:/* 1.0 Get Multiple Descriptors*/
		/* Copies one or more local descriptor table (LDT) entries into a
		 * client buffer.
		 *
		 * Call With:
		 *		AX			= 0x000e
		 *		CX			= number of descriptors to copy
		 *		ES:(E)DI	= selector:offset of a buffer in the following
		 *					  format
		 *					Offset	Length	Contents
		 *					00H		2		Selector #1 (set by client)
		 *					02H		8		Descriptor #1 (returned by host)
		 *					0AH		2		Selector #2 (set by client)
		 *					0CH		8		Descriptor #2 (returned by host)
		 *					.		.		.
		 *					.		.		.
		 *					.		.		.
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8022	invalid selector
		 *		CX			= number of descriptors successfully copied
		 *
		 * Notes:
		 *
		 * o	If an error occurs because of an invalid selector or
		 *		descriptor, the function returns the number of descriptors
		 *		which were successfully copied in CX.  All of the descriptors
		 *		which were copied prior to the one that failed are valid.
		 *
		 * o	32-bit programs must use ES:EDI to point to the buffer.
		 *		16-bit programs should use ES:DI.
		 */
	case 0x000f:/* 1.0 Set Multiple Descriptors*/
		/* Copies one or more descriptors from a client buffer into the local
		 * descriptor table (LDT).
		 *
		 * Call With:
		 *		AX			= 0x000f
		 *		CX			= number of descriptors to copy
		 *		ES:(E)DI	= selector:offset of a buffer in the following
		 *					  format
		 *					Offset	Length	Contents
		 *					00H		2		Selector #1
		 *					02H		8		Descriptor #1
		 *					0AH		2		Selector #2
		 *					0CH		8		Descriptor #2
		 *					.		.		.
		 *					.		.		.
		 *					.		.		.
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x8021	invalid value (access rights/type
		 *								bytes invalid)
		 *						0x8022	invalid selector
		 *						0x8025	invalid linear address (descriptor
		 *								reference a linear address range
		 *								outside that allowed for DPMI clients)
		 *		CX			= number of descriptors successfully copied
		 *
		 * Notes:
		 *
		 * o	If an error occurs because of an invalid selector or
		 *		descriptor, the function returns the number of descriptors
		 *		which were successfully copied in CX.  All of the descriptors
		 *		which were copied prior to the one that failed are valid.  All
		 *		descriptors from the invalid entry to the end of the table are
		 *		not updated.
		 *
		 * o	32-bit programs must use ES:EDI to point to the buffer.
		 *		16-bit programs should use ES:DI.
		 *
		 * o	The descriptor's access rights/type bytes (byte 5) follows the
		 *		same format and restrictions as the access rights/type
		 *		parameter (in CL) for the Set Descriptor Access Rights
		 *		function (int 31H Function 0009H).  On 80386 (or later)
		 *		machines, the descriptor's extended access rights/type byte
		 *		(byte 6) follows the same format and restrictions as the
		 *		extended axxess rights/type parameter (in CH) for the same
		 *		function, except the low-order 4 vits (marked "reserved") are
		 *		used to set the upper 4 bits of the descriptor's limit.
		 *
		 * o	If the descriptor's present bit is not set, then the only
		 *		error checking is that the client's CPL must be equal to the
		 *		descriptor's DPL field and the "must be 1" bit in the
		 *		descriptor's byte 5 must be set.
		 *
		 * o	A DPMI 1.0 host will reload any segment registers which
		 *		contain a selector specified in the data structure supplied to
		 *		this function. It is suggested that DPMI 0.9 hosts also
		 *		implement this.
		 */

	/* Extended Memory Management Services */
	case 0x0500:/* 0.9 Get Free Memory Information*/
		/*??
		 * Call With:
		 *		AX			= 0x0500
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0501:/* 0.9 Allocate Memory Block*/
		/*??
		 * Call With:
		 *		AX			= 0x0501
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0502:/* 0.9 Free Memory Block*/
		/*??
		 * Call With:
		 *		AX			= 0x0502
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0503:/* 0.9 Resize Memory Block*/
		/*??
		 * Call With:
		 *		AX			= 0x0503
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0504:/* 1.0 Allocate Linear Memory Block*/
		/*??
		 * Call With:
		 *		AX			= 0x0504
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0505:/* 1.0 Resize Linear Memory Block*/
		/*??
		 * Call With:
		 *		AX			= 0x0505
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0506:/* 1.0 Get Page Attributes*/
		/*??
		 * Call With:
		 *		AX			= 0x0506
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0507:/* 1.0 Set Page Attributes*/
		/*??
		 * Call With:
		 *		AX			= 0x0507
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0508:/* 1.0 Map Device in Memory Block*/
		/*??
		 * Call With:
		 *		AX			= 0x0508
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0509:/* 1.0 Map Conventional Memory in Memory Block*/
		/*??
		 * Call With:
		 *		AX			= 0x0509
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x050a:/* 1.0 Get Memory Block Size and Base*/
		/*??
		 * Call With:
		 *		AX			= 0x050a
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x050b:/* 1.0 Get Memory Information*/
		/*??
		 * Call With:
		 *		AX			= 0x050b
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0800:/* 0.9 Physical Address Mapping*/
		/*??
		 * Call With:
		 *		AX			= 0x0800
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0801:/* 1.0 Free Physical Address Mapping*/
		/*??
		 * Call With:
		 *		AX			= 0x0801
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0d00:/* 1.0 Allocate Shared Memory*/
		/*??
		 * Call With:
		 *		AX			= 0x0d00
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0d01:/* 1.0 Free Shared Memory*/
		/*??
		 * Call With:
		 *		AX			= 0x0d01
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0d02:/* 1.0 Serialize on Shared Memory*/
		/*??
		 * Call With:
		 *		AX			= 0x0d02
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0d03:/* 1.0 Free Serialization on Shared Memory*/
		/*??
		 * Call With:
		 *		AX			= 0x0d03
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */

	/* DOS Memory Management Services */
	case 0x0100:/* 0.9 Allocate DOS Memory Block*/
		/* Allocates a block of memory from the DOS memory pool, i.e. memroy
		 * below the 1 MB bondary that is controlled by DOS.  Such memory blocks
		 * are typically used to exchange data with real mode programs, TSRs,
		 * or device drivers.  The function returns both the real mode segment
		 * base address of the block and one or more descriptors that can be
		 * used by protected mode applications to access the block.
		 *
		 * Call With:
		 *		AX			= 0x0100
		 *		BX			= number of (16-byte) paragraphs desired
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *		AX			= real mode segment base address of allocated
		 *					  block
		 *		DX			= selector for allocated block
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x0007	memory control blocks damaged (also
		 *								returned by DPMI 0.9 hosts)
		 *						0x0008	insufficient memory (also returned by
		 *								DPMI 0.9 hosts)
		 *						0x8011	descriptor unavailable
		 *		BX			= size of largest available block in paragraphs
		 *
		 * Notes:
		 *
		 * o	If the size of the block requested is reater that 64 KB (BX >
		 *		1000H) and the client is a 16-bit program, contiguous
		 *		descriptors are allocated and the base selector is returned.
		 *		The consecutive selectors for the memory block can be
		 *		calcualted using the value returned by the Get Selector
		 *		Increment Value function (Int 31H Function 0003H). Each
		 *		descriptor has a limit of 64 KB, except for the last wich has
		 *		a limit of blocksize MOD 64 KB.
		 *
		 * o	If the DPMI host is 32-bit, the client is 16-bit, and more
		 *		than one descriptor is allocated, the limit of the first
		 *		descriptor will be set to the size of the entire block.
		 *		Subsequent descriptors have imits as described in the previous
		 *		Note.  16-bit DPMI hosts will always set the limit of the
		 *		first descriptor to 64 KB even when running on an 80386 (or
		 *		later) machine.
		 *
		 * o	When the client is 32-bit, this function always allocates only
		 *		one descriptor.
		 *
		 * o	Client programs should never modifiy or free any descriptors
		 *		allocated by this function. The Free DOS Memory Block function
		 *		(Int 31H function 0101H) will deallocate the descriptors
		 *		automatically.
		 *
		 * o	The DOS alloocation function (Int 21H Funciton 48H) is used.
		 */
	case 0x0101:/* 0.9 Free DOS Memory Block*/
		/* Frees a memory block that was previously allocated with the
		 * Allocate DOS Memory Block function (Int 31H Function 0100H).
		 * Call With:
		 *		AX			= 0x0101
		 *		DX			= selector of block to be freed
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x0007	memory control blocks damaged (also
		 *								returned by DPMI 0.9 hosts)
		 *						0x0009	in correct memory segment specified
		 *								(also returned by DPMI 0.9 hosts)
		 *						0x8022	invalid selector
		 *
		 * Notes:
		 *
		 * o	All descriptors allocated for the memory block are
		 *		automatically freed by this function, and are no longer valid
		 *		after this function returns.
		 *
		 * o	Under DPMI 1.0 hosts, any segment registers which contain a
		 *		selector being freed are zeroed by this function.
		 */
	case 0x0102:/* 0.9 Resize DOS Memory Block*/
		/* Changes the size of a memory block that was previously allocated
		 * with the Allocate DOS Memory Block function (Int 31H Function
		 * 0100H).
		 * Call With:
		 *		AX			= 0x0102
		 *		BX			= new block size in (16-byte) paragraphs
		 *		DX			= selector of block to modify
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *						0x0007	memory control blocks damaged (also
		 *								returned by DPMI 0.9 hosts)
		 *						0x0008	insufficient memory (also returned by
		 *								DPMI 0.9 hosts)
		 *						0x0009	in correct memory segment specified
		 *								(also returned by DPMI 0.9 hosts)
		 *						0x8011	descriptor unavailable
		 *						0x8022	invalid selector
		 *
		 * Notes:
		 *
		 * o	Requests to increase the size of an existing DOS memory block
		 *		may fail due to subsequent DOS memory block allocations
		 *		causing fragmentation of DOS memory, or insufficient remaining
		 *		DOS memory.  In addition, the function will fail if the block
		 *		is growing past a 64 KB boundary and the next descriptor in
		 *		the LDT is not available.
		 *
		 * o	A request to decrease the size of a DOS memory block may cause
		 *		some descriptors that were previously allocated to the block
		 *		to be freed and the limit of the new last descriptor for the
		 *		block to be changed.
		 *
		 * o	Under a DPMI 1.0 host, any segment registers which contain a
		 *		selector being modified are reloaded by this function and any
		 *		segment registers which contain a selector being freed are
		 *		zeroed by this function.
		 *
		 * o	Client programs should never modifiy or free any descriptors
		 *		allocated by this function. The Free DOS Memory Block function
		 *		(Int 31H function 0101H) will deallocate the descriptors
		 *		automatically.
		 */

	/* Interrupt Management Services */
	case 0x0200:/* 0.9 Get Real Mode Interrupt Vector*/
		/*??
		 * Call With:
		 *		AX			= 0x0200
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0201:/* 0.9 Set Real Mode Interrupt Vector*/
		/*??
		 * Call With:
		 *		AX			= 0x0201
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0202:/* 0.9 Get Processor Exception Handler Vector*/
		/*??
		 * Call With:
		 *		AX			= 0x0202
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0203:/* 0.9 Set Processor Exception Handler Vector*/
		/*??
		 * Call With:
		 *		AX			= 0x0203
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0204:/* 0.9 Get Protected Mode Interrupt Vector*/
		/*??
		 * Call With:
		 *		AX			= 0x0204
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0205:/* 0.9 Set Protected Mode Interrupt Vector*/
		/*??
		 * Call With:
		 *		AX			= 0x0205
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0210:/* 1.0 Get Extended Processor Exception Handle In Protected Mode*/
		/*??
		 * Call With:
		 *		AX			= 0x0210
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0211:/* 1.0 Get Extended Processor Exception Handle In Real Mode*/
		/*??
		 * Call With:
		 *		AX			= 0x0211
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0212:/* 1.0 Set Extended Processor Exception Handle In Protected Mode*/
		/*??
		 * Call With:
		 *		AX			= 0x0212
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0213:/* 1.0 Set Extended Processor Exception Handle In Real Mode*/
		/*??
		 * Call With:
		 *		AX			= 0x0213
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0900:/* 0.9 Get and Disable Virtual Interrupt State*/
		/*??
		 * Call With:
		 *		AX			= 0x0900
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0901:/* 0.9 Get and Enable Virtual Interrupt State*/
		/*??
		 * Call With:
		 *		AX			= 0x0901
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0902:/* 0.9 Get virtual Interrupt State*/
		/*??
		 * Call With:
		 *		AX			= 0x0902
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */

	/* Translation Services */
	case 0x0300:/* 0.9 Simulate Real Mode Interrupt*/
		/*??
		 * Call With:
		 *		AX			= 0x0300
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0301:/* 0.9 Call Real Mode Procedure with Far Return Frame*/
		/*??
		 * Call With:
		 *		AX			= 0x0301
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0302:/* 0.9 Call Real Mode Procedure with Interrupt Return Frame*/
		/*??
		 * Call With:
		 *		AX			= 0x0302
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0303:/* 0.9 Allocate Real Mode Callback Address*/
		/*??
		 * Call With:
		 *		AX			= 0x0303
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0304:/* 0.9 Free Real Mode Callback Address*/
		/*??
		 * Call With:
		 *		AX			= 0x0304
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0305:/* 0.9 Get State Save/Restore Addresses*/
		/*??
		 * Call With:
		 *		AX			= 0x0305
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0306:/* 0.9 Get Raw CPU Mode Switch Addresses*/
		/*??
		 * Call With:
		 *		AX			= 0x0306
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */

	/* Page Management Services */
	case 0x0600:/* 0.9 Lock Linear Region*/
		/*??
		 * Call With:
		 *		AX			= 0x0600
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0601:/* 0.9 Unlock Linear Region*/
		/*??
		 * Call With:
		 *		AX			= 0x0601
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0602:/* 0.9 Mark Real Mode Region as Pageable*/
		/*??
		 * Call With:
		 *		AX			= 0x0602
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0603:/* 0.9 Relock Real Mode Region*/
		/*??
		 * Call With:
		 *		AX			= 0x0603
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0604:/* 0.9 Get Page Size*/
		/*??
		 * Call With:
		 *		AX			= 0x0604
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0700:/*reserved*/
		/* DPMI Function 0700H is reserved for historical reasons and should
		 * not be called.
		 *
		 * undocumented (from Ralf Brown's interrupt list)
--------E-310700-----------------------------
INT 31 Pu - DPMI 0.9+ - MARK PAGES AS PAGING CANDIDATES
		AX = 0700h
		BX:CX = starting linear page number
		SI:DI = number of pages to mark as paging candidates
Return: unknown
Note:	although marked as reserved in versions 0.9 and 1.0 of the DPMI
		  specification, this function is called by MS Windows TASKMAN,
		  PROGMAN, and KERNEL
		 *
		 *
		 * Call With:
		 *		AX			= 0x0700
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0701:/*reserved*/
		/* DPMI Function 0701H is reserved for historical reasons and should
		 * not be called.
		 *
		 * undocumented (from Ralf Brown's interrupt list)
--------E-310701-----------------------------
INT 31 Pu - DPMI 0.9+ - DISCARD PAGES
		AX = 0701h
		BX:CX = starting linear page number
		SI:DI = number of pages to discard
Return: unknown
Note:	although marked as reserved in versions 0.9 and 1.0 of the DPMI
		  specification, this function is called by MS Windows TASKMAN,
		  PROGMAN, and KERNEL
		 *
		 * Call With:
		 *		AX			= 0x0701
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0702:/* 0.9 Mark Page as Demand Paging Candidate*/
		/*??
		 * Call With:
		 *		AX			= 0x0702
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0703:/* 0.9 Discard Page Contents*/
		/*??
		 * Call With:
		 *		AX			= 0x0703
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */

	/* Debug Support Services */
	case 0x0b00:/* 0.9 Set Debug Watchpoint*/
		/*??
		 * Call With:
		 *		AX			= 0x0b00
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0b01:/* 0.9 Clear Debug Watchpoint*/
		/*??
		 * Call With:
		 *		AX			= 0x0b01
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0b02:/* 0.9 Get State of Debug Watchpoint*/
		/*??
		 * Call With:
		 *		AX			= 0x0b02
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0b03:/* 0.9 Reset Debug Watchpoint*/
		/*??
		 * Call With:
		 *		AX			= 0x0b03
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */

	/* Miscellaneous Services */
	case 0x0400:/* 0.9 Get DPMI Version*/
		/*??
		 * Call With:
		 *		AX			= 0x0400
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0401:/* 1.0 Get DPMI Capabilities*/
		/*??
		 * Call With:
		 *		AX			= 0x0401
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0a00:/* 0.9 Get Vendor-Specific API Entry Point*/
		/*??
		 * Call With:
		 *		AX			= 0x0a00
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0c00:/* 1.0 Install Resident Service Provider Callback*/
		/*??
		 * Call With:
		 *		AX			= 0x0c00
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0c01:/* 1.0 Terminate and Stay Resident*/
		/*??
		 * Call With:
		 *		AX			= 0x0c01
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0e00:/* 1.0 Get Coprocessor Status*/
		/*??
		 * Call With:
		 *		AX			= 0x0e00
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */
	case 0x0e01:/* 1.0 Set Coprocessor Emulation*/
		/*??
		 * Call With:
		 *		AX			= 0x0e01
		 *
		 * Returns:
		 *
		 * if function successful
		 *		Carry flag	= clear
		 *
		 * if function unsuccessful
		 *		Carry flag	= set
		 *		AX			= error code
		 *
		 * Notes:
		 *
		 */

	default:/* Illegal Function Code */
		return 0x8001;
	}
}
