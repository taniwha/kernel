OUTPUT_FORMAT("coff-go32")
ENTRY(start)
SECTIONS
{
  .init  0x1000+SIZEOF_HEADERS : {
	*(.init)
	. = ALIGN(0x1000);
	einit = . ;
  }
  .text  0xff800000 : { /* this limits the kernel code and data to 4MB (see physmem.c) */
    stext = . ;
    *(.text)
    etext  =  . ; _etext = .;
    . = ALIGN(0x200);
  }
  .data  ALIGN(0x200) : {
    djgpp_first_ctor = . ;
    *(.ctor)
    djgpp_last_ctor = . ;
    djgpp_first_dtor = . ;
    *(.dtor)
    djgpp_last_dtor = . ;
    *(.data)
     edata  =  . ; _edata = .;
     . = ALIGN(0x200);
  }
  .bss  SIZEOF(.data) + ADDR(.data) :
  { 					
    *(.bss)
    *(COMMON)
     . = ALIGN(0x1000);
     /* guarantee 16k of stack space */
     bottom_of_stack = .;
     . += 16384;
     top_of_stack = .;
     end = . ; _end = .;
     . = ALIGN(0x200);
  }
}
