#define FDC_DOR(base)	((base)+2)
#define FDC_MSR(base)	((base)+4)
#define FDC_DSR(base)	((base)+4)
#define FDC_FIFO(base)	((base)+5)
#define FDC_DIR(base)	((base)+7)
#define FDC_CCR(base)	((base)+7)

int fdc_seek(int drive, int cyl, int head);
int fdc_read(int drive, int cyl, int head, int sec, int cnt, void *buff);
int fdc_write(int drive, int cyl, int head, int sec, int cnt, void *buff);
