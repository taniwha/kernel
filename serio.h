#ifndef __serio_h
#define __serio_h

#ifdef __cplusplus
extern "C" {
#endif

enum sioWordSize {
	sio5Bits,
	sio6Bits,
	sio7Bits,
	sio8Bits,
};

enum sioParity {
	sioNoParity		=0x00,
	sioOddParity	=0x08,
	sioEvenParity	=0x18,
	sioMarkParity	=0x28,
	sioSpaceParity	=0x38,
};

enum sioStopBits {
	sio1StopBit=0x00,
	sio1_5StopBits=0x04,
	sio2StopBits=0x04,
};

#ifndef __sio_do_not_define_function_prototypes_or_externs

typedef struct {
	volatile int sio_error;		/* a reveive error has occured */
	volatile int sio_errct;		/* number of receive errors */
	volatile int sio_chars_sent;
	volatile char sio_break;	/* a break has been detected */
	volatile char sio_linestat;	/* current line status */
	volatile char sio_modemstat;/* current modem status */
	int sio_doxoff;				/* enable s/w flow control */
	int sio_brkmode;			/* enable break detection */
} SioPort;

int  sio_put(SioPort *port, char characterToSend);	/* 0=success, 1=buffer full */
int  sio_write(SioPort *port, void *data, int len);	/* number of bytes placed in xmit buffer*/
int  sio_charready(SioPort *port);					/* 1 if chars avail, 0 if nothing there */
int  sio_get(SioPort *port);						/* -1 if break detected, char otherwise */
void sio_sendbreak(SioPort *port, int charactersToHoldBreak);
void sio_setspeed(SioPort *port, int baudRateDivisor);	/* does not convert from baud */
int  sio_getspeed(SioPort *port);					/* does not convert to baud */
void sio_setmcr(SioPort *port, int valueToSetModemControlRegisterTo);
int  sio_getmcr(SioPort *port);
void sio_setlcr(SioPort *port, int valueToSetLineControlRegisterTo);
int  sio_getlcr(SioPort *port);
void sio_setparms(SioPort *port, enum sioWordSize, enum sioParity, enum sioStopBits);
SioPort *sio_openport(int baseAddress, int hardwareInterruptRequestNumber);
void sio_closeport(SioPort *port);

static __inline__
int sio_error(SioPort *port)
{
	int ret=0;
	asm ("xchgl %0,%1":"=r"(ret):"o"(port->sio_error),"0"(ret));
	return ret;
}

static __inline__
int sio_errct(SioPort *port)
{
	int ret=0;
	asm ("xchgl %0,%1":"=r"(ret):"o"(port->sio_errct),"0"(ret));
	return ret;
}

static __inline__
int sio_chars_sent(SioPort *port)
{
	int ret=0;
	asm ("xchgl %0,%1":"=r"(ret):"o"(port->sio_chars_sent),"0"(ret));
	return ret;
}

static __inline__
int sio_break(SioPort *port)
{
	int ret=0;
	asm ("xchgl %0,%1":"=r"(ret):"o"(port->sio_break),"0"(ret));
	return ret;
}

static __inline__
int sio_linestat(SioPort *port)
{
	return port->sio_linestat;
}

static __inline__
int sio_modemstat(SioPort *port)
{
	return port->sio_modemstat;
}

#endif//__sio_do_not_define_function_prototypes_or_externs

#ifdef __cplusplus
}
#endif

#endif//__serio_h
