/* 
 *  Replacements for the C library malloc routines, based on the malloc
 *  from the Atari ST gcc libc, which in turn was based on Dave  
 *  Schumakers dlibs routines.
 * 
 *  Shawn Hargreaves, shawn@talula.demon.co.uk
 */

#if 0
#include <stdlib.h>
#include <libc/stubs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

/* how much space to reserve on each call to sbrk() */
#define GROW_MARGIN  1024*256

#include "types.h"
#include "kmalloc.h"
#include "screen.h"
#define getpagesize() 4096
#define NULL 0
void *ksbrk(int delta);

void *memcpy(void *dst, const void *src, ulong len)
{
	void *d=dst;
	const void *s=src;
	ulong l=len/4;

	if (dst>src) {
		s+=len;
		d+=len;
		asm volatile ("std");
		if (len&1) {
			asm volatile ("movsb":"=S"(s),"=D"(d):"0"(s),"1"(d));
		}
		if (len&2) {
			asm volatile ("movsw":"=S"(s),"=D"(d):"0"(s),"1"(d));
		}
		asm volatile ("rep;movsl;cld":"=S"(s),"=D"(d),"=c"(l):"0"(s),"1"(d),"c"(l));
	} else {
		asm volatile ("cld;rep;movsl":"=S"(s),"=D"(d),"=c"(l):"0"(s),"1"(d),"c"(l));
		if (len&2) {
			asm volatile ("movsw":"=S"(s),"=D"(d):"0"(s),"1"(d));
		}
		if (len&1) {
			asm volatile ("movsb":"=S"(s),"=D"(d):"0"(s),"1"(d));
		}
	}
	return dst;
}

void *memset(void *dst, int val, ulong len)
{
	void *d=dst;
	ulong v;
	ulong l=len/4;

	asm volatile ("
		movb	%%dl,%%dh
		movw	%%dx,%%ax
		shll	$16,%%eax
		movw	%%dx,%%ax
		"
		:"=a"(v)
		:"d"(val)
		:"%edx"
	);
	asm volatile ("cld;rep;stosl":"=D"(d),"=a"(v),"=c"(l):"0"(d),"1"(v),"2"(l));
	if (len&2) {
		asm volatile ("stosw":"=D"(d),"=a"(v):"0"(d),"1"(v));
	}
	if (len&1) {
		asm volatile ("stosb":"=D"(d),"=a"(v):"0"(d),"1"(v));
	}
	return dst;
}


/* linked list of memory chunks */
typedef struct MemChunk
{
   struct MemChunk *next;
   unsigned long size;
} MemChunk;



/* list of free chunks */
static MemChunk the_heap = { NULL, 0 };



/* malloc:
 *  Allocates n bytes of memory, and returns a pointer to it. 
 */
void *kmalloc(ulong n)
{
   MemChunk *p, *q;

   static int virgin = 1;

   /* first time we are called, align the break point */
   if (virgin) {
      int pagesize, x;
      void *p;

      pagesize = x = getpagesize();
	  p = ksbrk(0);

      x = (x - ((int)p & (x - 1))) & (x-1);
      if (x < 0)
         x += pagesize;

      p=ksbrk(x);
      virgin = 0;
   }

   /* add sizeof(MemChunk) to the requested size, and round up */
   n += sizeof(MemChunk);
   n = (n+7) & ~7;

   do {
      /* look for a big enough block in the free list */
      p = &the_heap;
      q = the_heap.next;

      while ((q) && (q->size < n)) {
         p = q;
         q = q->next;
      }

      /* if not enough memory, expand the heap */
      if (!q) {
         int grow = (n + (GROW_MARGIN-1)) & ~(GROW_MARGIN-1);
         p = ksbrk(grow);
         if (p == (MemChunk *)-1)
            return NULL;

         p->size = grow;
         kfree(p+1);
      }

   } while (!q);
   if (q->size > n + sizeof(MemChunk)) {
	  /* split this block, leaving part of it in the free list */
	  p->next = (MemChunk *)(((long)q) + n);
	  p->next->size = q->size - n;
	  p->next->next = q->next;
      q->size = n;
   }
   else {
      /* unlink the entire chunk */
      p->next = q->next;
   }

   /* skip past the block size information */
   q->next = NULL; 

   q++;
   return ((void *)q);
}



/* realloc:
 *  Changes the size of a block of memory previously allocated by 
 *  malloc().
 */
void *krealloc(void *_r, ulong n)
{
   MemChunk *s, *t, *p, *q, *r = _r;
   long sz;

   /* obscure features: realloc(NULL,n) is the same as malloc(n)
    *                   realloc(p, 0) is the same as free(p)
    */
   if (!r)
      return kmalloc(n);

   if (!n) {
      kfree(_r);
      return NULL;
   }

   p = r - 1;
   sz = (n + sizeof(MemChunk)+7) & ~7;

   if (p->size > sz) { 
      /* block too big - split it in two */
      q = (MemChunk *)(((long)p) + sz);
      q->size = p->size - sz;
      kfree(q+1);
      p->size = sz;
   }
   else {
      if (p->size < sz) { 
         /* block too small, need to expand it */
         q = &the_heap;
         t = the_heap.next;

         while ((t) && (t<p)) {
            q = t;
            t = t->next;
         }

         s = (MemChunk *)(((long)p) + p->size);
         if ((t) && (s == t) && (p->size + t->size >= sz)) {
            /* expand the block in-situ */
            p->size += t->size;
            q->next = t->next;
            t->size = 0;
            t->next = NULL;
         }
         else {
            /* argh! have to move the block */
            q = (MemChunk *)kmalloc(n);
            if (!q)
               return NULL;

            n = p->size - sizeof(MemChunk);
            memcpy(q, r, n);
            kfree(r);
            r = q;
         }
      }
   }

   return ((void *)r);
}



/* free:
 *  Frees a block of memory previously allocated by malloc().
 */
void kfree(void *_r)
{
   MemChunk *p, *q, *s;
   MemChunk *r = (MemChunk *)_r;

   if (!r)
      return;

   /* move back to uncover the MemChunk structure */
   r--;

   /* insert into free list, sorting by address */
   p = &the_heap;
   q = the_heap.next;

   while ((q) && (q<r)) {
      p = q;
      q = q->next;
   }

   /* merge with the following block if possible */
   s = (MemChunk *)(((long)r) + r->size);
   if ((q) && (s == q)) {
      r->size += q->size;
      q = q->next;
      s->size = 0;
      s->next = NULL;
   }
   r->next = q;

   /* merge with the preceding block if possible, otherwise link it */
   s = (MemChunk *)(((long)p) + p->size);
   if ((s == r) && (p != &the_heap)) {
      p->size += r->size;
      p->next = r->next;
      r->size = 0;
      r->next = NULL;
   }
   else
      p->next = r;
}



/* __heap_trace:
 *  Diagnostic routine, prints free memory blocks to stdout.
 */
void __heap_trace(void)
{
    MemChunk *p = the_heap.next;

    while (p) {
        kprintf("Free block at 0x%x, size %d\n", (int)p, p->size);
        p = p->next;
    }
}


/*
 *  Shawn Hargreaves - shawn@talula.demon.co.uk - http://www.talula.demon.co.uk/
 *  Ghoti: 'gh' as in 'enough', 'o' as in 'women', and 'ti' as in 'nation'.
 */
