/* This code was provided by nobody <ahoffmey@mail.pressenter.com (Andy, I beleive)
 */
#include "keybdio.h"
#include "screen.h"

void kgets(char *buf, int max)
    {
    union TKeyData c;
    char key;
    int pos=0;
    do
        {
        while (!(c.keyData=k_getKey()));
        key=c.codes.charCode;
        if (c.keyData==0) continue;
        switch (key) {
            case 13:
                {
                buf[pos]=0;
                break;
                }
            case '\b':
                {
                if (pos==0) break;
                else
                    {
                    pos--;
                    kputc('\b');
                    }
                break;
                }
            default:
				{
				if (pos<max-1) {
					buf[pos]=key;
					kputc(key);
					pos++;
					break;
				}
                }
            }
        }
    while (key!=13);
    }
