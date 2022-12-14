void inton(void);
void intoff(void);
void lidt(void *base, short limit);
void lgdt(void *base, short limit);
void lldt(short selector);
void ltr(short selector);
void jmptss(short selector);
void clearnt(void);
void lcr3(long cr3);
void lcr0(long cr0);
void outb(short port, char data);
void outw(short port, short data);
void outl(short port, long data);
unsigned char inb(short port);
unsigned short inw(short port);
unsigned long inl(short port);
void outsb(short port, void *data, long count);
void outsw(short port, void *data, long count);
void outsl(short port, void *data, long count);
void insb(short port, void *data, long count);
void insw(short port, void *data, long count);
void insl(short port, void *data, long count);
