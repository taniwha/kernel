#include "types.h"
#include "screen.h"
#include "isa.h"
#include "dma.h"
#include "s386.h"
#include "fdc.h"
#include "cmos.h"

#define TIMEOUT 1000000

static uchar fdc_type[2];
static uchar fdc_speed[2];
static uchar fdc_known[2];
static uchar fdc_curcyl[2];
static uchar fdc_rate;

inline static int write_byte(int byte)
{
	int timeout=TIMEOUT;
	while (--timeout) {
		if ((inb(FDC_MSR(FDC1))&0xc0)==0x80) {
			outb(FDC_FIFO(FDC1),byte);
			return 0;
		}
	}
	return 0x80;	/* timeout */
}

inline static int read_byte(void)
{
	int timeout=TIMEOUT;
	while (--timeout) {
		if ((inb(FDC_MSR(FDC1))&0xc0)==0xc0) {
			return inb(FDC_FIFO(FDC1));
		}
	}
	return -1;
}

static int write_command(int cnt, uchar *cmd)
{
	while (cnt--) {
		int err;
		if ((err=write_byte(*cmd++))) {
			return err;
		}
	}
	return 0;
}

static int read_data(int cnt, uchar *data)
{
	while (cnt--) {
		int dat=read_byte();
		if (dat<0) {
			return 0x80;	/* timeout */
		}
		*data++=dat;
	}
	return 0;
}

static void select_drive(int drive)
{
	outb(FDC_DOR(FDC1),(0x10<<drive)|0x0c|drive);
}

static volatile int done;

static void fdc_isr(void)
{
	done=1;
}

static void __attribute__((constructor)) fdc_init(void)
{
	uchar b;

	set_irq_vector(6,fdc_isr);
	outb(ICU1+1,inb(ICU1+1)&0xbf);
	outb(FDC_CCR(FDC1),0);			/* set for 500kbps data rate */
	b=read_cmos(0x90);
	fdc_type[0]=b>>4;
	fdc_type[1]=b&0xf;
}

static void wait_for_done(void)
{
	while (!done) {
		static int tick;
		static char d[]={0x2f,0x2d,0x5c,0x7c};
		kputc(d[tick++%4]);
		kputc('\b');
	};
}

static int seek(int drive, int cyl, int head)
{
	int err;
	uchar res[2];
	uchar cmd[]={0x0f,((head&1)<<2)|(drive&3),cyl};

	done=0;
	if ((err=write_command(sizeof(cmd),cmd)))
		return err;
	/*if ((err=*/wait_for_done()/*))
		return err*/;
	if ((err=write_byte(0x08)))
		return err;
	if ((err=read_data(sizeof(res),res)))
		return err;
	if ((res[0]&0x60)==0x60)
		return 0x40;	/* seek failed */
	return 0;
}

static int parse_result(const uchar *res)
{
	if (!(res[0]&0xc0)) return 0x00;	/* no error */
	if (!(res[0]&0x20)) return 0x20;	/* controller failure */
	if (res[1]&0x01) return 0x02;		/* address mark not found */
	if (res[1]&0x02) return 0x03;		/* write protected */
	if (res[1]&0x04) return 0x04;		/* sector not found/read error */
	if (res[1]&0x10) return 0x08;		/* DMA overrun */
	if (res[1]&0x20) return 0x10;		/* uncorrectable CRC or ECC error on read */
	if (res[1]&0x80) return 0x04;		/* sector not found/read error */
	return 0x20;						/* controller failure */
}

static int read_id(int drive)
{
	int err;
	uchar res[7];
	uchar cmd[]={0x4a,drive&3};

	done=0;
	if ((err=write_command(sizeof(cmd),cmd)))
		return err;
	/*if ((err=*/wait_for_done()/*))
		return err*/;
	if ((err=read_data(sizeof(res),res)))
		return err;
	return parse_result(res);
}

int fdc_recalibrate(int drive)
{
	int err;
	uchar res[2];
	uchar cmd[]={0x07,drive&3};

	done=0;
	if ((err=write_command(sizeof(cmd),cmd)))
		return err;
	/*if ((err=*/wait_for_done()/*))
		return err*/;
	if ((err=write_byte(0x08)))
		return err;
	if ((err=read_data(sizeof(res),res)))
		return err;
	if ((res[0]&0x60)==0x60)
		return 0x40;	/* seek failed */
	fdc_curcyl[drive]=0;
	return 0;
}

static int select_speed(int drive)
{
	int err;

	if (inb(FDC_DIR(FDC1))&0x80) {
		fdc_known[drive]=0;
		/*???*/
		if ((err=seek(drive,1,0)))
			return err;
		if ((err=seek(drive,0,0)))
			return err;
		if (inb(FDC_DIR(FDC1))&0x80)
			return 0x20;		/* controller failure */
		return 0x06;			/* disk changed */
	}
	if (!fdc_known[drive]) {
		fdc_recalibrate(drive);
		switch (fdc_type[drive]) {
		case 1:	/* 360k drive */
		case 3:	/* 720k drive */
			fdc_speed[drive]=2;		/* 250 kbps */
			fdc_rate=fdc_speed[drive];
			outb(FDC_CCR(FDC1),fdc_speed[drive]);
			if (read_id(drive)==0) {
				fdc_known[drive]=1;
				return 0;
			}
			return 0x31;			/* no media in drive or bad track 0 */
		case 2:	/* 1200k drive */
			fdc_speed[drive]=0;		/* 500 kbps */
			fdc_rate=fdc_speed[drive];
			outb(FDC_CCR(FDC1),fdc_speed[drive]);
			if (read_id(drive)==0) {
				fdc_known[drive]=1;
				return 0;
			}
			fdc_speed[drive]=1;		/* 300 kbps */
			fdc_rate=fdc_speed[drive];
			outb(FDC_CCR(FDC1),fdc_speed[drive]);
			if (read_id(drive)==0) {
				fdc_known[drive]=1;
				return 0;
			}
			return 0x31;			/* no media in drive or bad track 0 */
		case 4: /* 1440k drive */
			fdc_speed[drive]=0;		/* 500 kbps */
			fdc_rate=fdc_speed[drive];
			outb(FDC_CCR(FDC1),fdc_speed[drive]);
			if (read_id(drive)==0) {
				fdc_known[drive]=1;
				return 0;
			}
			fdc_speed[drive]=2;		/* 250 kbps */
			fdc_rate=fdc_speed[drive];
			outb(FDC_CCR(FDC1),fdc_speed[drive]);
			if (read_id(drive)==0) {
				fdc_known[drive]=1;
				return 0;
			}
			return 0x31;			/* no media in drive or bad track 0 */
		}
	}
	if (fdc_rate!=fdc_speed[drive]) {
		fdc_rate=fdc_speed[drive];
		outb(FDC_CCR(FDC1),fdc_speed[drive]);
	}
	return 0;
}
/*
 05h	data did not verify correctly (TI Professional PC)
 06h	disk changed (floppy)
 0Ch	unsupported track or invalid media
 31h	no media in drive (IBM/MS INT 13 extensions)
 80h	timeout (not ready)
*/

static int read_write(int wrt, int drv, int cyl, int head, int sec, int cnt, void *buff)
{
	uchar cmd[]={
		wrt?0xc5:0xe6,
		((head&1)<<2)|(drv&3),
		cyl, head, sec,
		2, 18, 27, 0xff	/* XXX these will need to be set correctly */
	};
	uchar res[7];
	int err;

	done=0;
	if (drv>1)
		return 0x01;	/* invalid parameter */
	select_drive(drv);
	if ((err=select_speed(drv)))
		return err;
	if (cyl!=fdc_curcyl[drv]) {
		if ((err=seek(drv,cyl,head)))
			return err;
	}
	if (dma_setup(2,buff,cnt*512,wrt)<0)
		return 0x09;	/* data boundary error */
	if ((err=write_command(sizeof(cmd),cmd)))
		return err;
	/*if ((err=*/wait_for_done()/*))
		return err*/;
	if ((err=read_data(sizeof(res),res)))
		return err;
	return parse_result(res);
}

int fdc_read(int drive, int cyl, int head, int sec, int cnt, void *buff)
{
	return read_write(0,drive,cyl,head,sec,cnt,buff);
}

int fdc_write(int drive, int cyl, int head, int sec, int cnt, void *buff)
{
	return read_write(1,drive,cyl,head,sec,cnt,buff);
}

int fdc_seek(int drive, int cyl, int head)
{
	if (drive>1)
		return 0x01;	/* invalid parameter */
	select_drive(drive);
	return seek(drive,cyl,head);
}
