#define ICU1	0x20
#define ICU2	0xa0

#define FDC1	0x3f0
#define FDC2	0x370

#define IDE1	0x1f0
#define IDE2	0x170

void (*get_irq_vector(int ivec))(void);
void set_irq_vector(int ivec, void (*vec)(void));
void enable_irq(uchar irq);
void disable_irq(uchar irq);
