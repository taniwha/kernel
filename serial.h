#include "serio.h"

#ifdef __cplusplus
extern "C" {
#ifndef __dj_include_malloc_h_
#include <malloc.h>
#endif
#ifndef DXE_MAGIC
#include <sys/dxe.h>
#endif
#ifndef __dj_include_sys_exceptn_h__
#include <sys/exceptn.h>
#endif
}

class SioComPort {
	// NOTE! do not change the order of these declarations, or your computer
	// will explode! They refer to the pointers setup in `serial.s'.
	SioPort *port;
	SioComPort(const SioComPort&);				// not allowed!!!
	SioComPort &operator=(const SioComPort&);	// not allowed!!!
public:
	SioComPort(int baseAddress, int hardwareInterruptRequestNumber)
	{
		port=sio_openport(baseAddress,hardwareInterruptRequestNumber);
	}
	~SioComPort()
	{
		sio_closeport(port);
	}
	int put(char characterToSend)
	{
		return sio_put(port,characterToSend);
	}
	int charready(void)
	{
		return sio_charready(port);
	}
	int get(void)
	{
		return sio_get(port);
	}
	void sendbreak(int charactersToHoldBreak)
	{
		sio_sendbreak(port,charactersToHoldBreak);
	}
	void setspeed(int baudRateDivisor)
	{
		sio_setspeed(port,baudRateDivisor);
	}
	int getspeed(void)
	{
		return sio_getspeed(port);
	}
	void setmcr(int valueToSetModemControlRegisterTo)
	{
		sio_setmcr(port,valueToSetModemControlRegisterTo);
	}
	int getmcr(void)
	{
		return sio_getmcr(port);
	}
	void setlcr(int valueToSetLineControlRegisterTo)
	{
		sio_setlcr(port,valueToSetLineControlRegisterTo);
	}
	int getlcr(void)
	{
		return sio_getlcr(port);
	}
	void setparms(enum sioWordSize wordSize, enum sioParity parity,
				  enum sioStopBits stopBits)
	{
		sio_setparms(port,wordSize,parity,stopBits);
	}
	int error()
	{
		int ret=0;
		asm ("xchgl %0,%1":"=r"(ret):"o"(port->sio_error),"0"(ret));
		return ret;
	}
	int errct()
	{
		int ret=0;
		asm ("xchgl %0,%1":"=r"(ret):"o"(port->sio_errct),"0"(ret));
		return ret;
	}
	int chars_sent()
	{
		int ret=0;
		asm ("xchgl %0,%1":"=r"(ret):"o"(port->sio_chars_sent),"0"(ret));
		return ret;
	}
	int sawbreak()
	{
		int ret=0;
		asm ("xchgl %0,%1":"=r"(ret):"o"(port->sio_break),"0"(ret));
		return ret;
	}
	int linestat()
	{
		return port->sio_linestat;
	}
	int modemstat()
	{
		return port->sio_modemstat;
	}
	int &doxoff()
	{
		return port->sio_doxoff;
	}
	int &brkmode()
	{
		return port->sio_brkmode;
	}
};

#else

#error This header file needs C++

#endif
