int dma_setup(int channel, void *buffer, unsigned long len, int dir);
int dma_reset(int channel);
int dma_pause(int channel);
int dma_resume(int channel);
unsigned long dma_get_count(int channel);
unsigned long dma_get_base(int channel);
int dma_get_status(int channel);

extern int dma_errno;
extern char *dma_errlist[];
