void p_free_pages(long page, long count);
long p_allocate_pages(long count, int contiguous);
ulong linear_to_physical(void *lin);
ulong physical_to_linear(ulong phys);
int map_memory(ulong lin, ulong phys, ushort flags, ulong len);

extern ulong pageTables[1047552];		/* 1M - 1024 */
extern ulong pageTableDirectory[1024];
