void *kmalloc(ulong n);
void *krealloc(void *_r, ulong n);
void kfree(void *_r);
void __heap_trace(void);

void *memcpy(void *dst, const void *src, ulong len);
void *memset(void *dst, int val, ulong len);
