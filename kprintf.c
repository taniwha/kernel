/* This code was provided by nobody <ahoffmey@mail.pressenter.com (Andy, I beleive)
 */
#include <stdarg.h>
#include "types.h"
#include "screen.h"

int kprintf(const char *fmt,...)
    {
    va_list arglist;
    int x=0;
	int dec;
	ulong hex;
    char *str;
    va_start(arglist,fmt);
    while (fmt[x]!=0)
        {
        if (fmt[x]=='%')
            {
            x++;
            switch (fmt[x])
                {
                case '%': {
                    kputc('%');
                    break;
                    }
                case 's': {
                    str=va_arg(arglist,char*);
                    kputs(str);
                    break;
                    }
                case 'd': {
                    dec=va_arg(arglist,int);
                    kputd(dec);
                    break;
                    }
                case 'c': {
                    dec=va_arg(arglist,char);
                    kputc(dec);
                    break;
                    }
				case 'h':
					if (fmt[x+1]!='x') {
						kputc(fmt[x]);
						break;
					}
					x++;
					hex=va_arg(arglist,unsigned short);
					kputsx(hex);
					break;
				case 'b':
					if (fmt[x+1]!='x') {
						kputc(fmt[x]);
						break;
					}
					x++;
					hex=va_arg(arglist,unsigned short);
					kputcx(hex);
					break;
                case 'x': {
                    hex=va_arg(arglist,unsigned long);
                    kputx(hex);
                    break;
					}
                default: {
                    kputc(fmt[x]);
                    break;
                    }
                }
            }
        else kputc(fmt[x]);
        x++;
        }
    return x;
    }
