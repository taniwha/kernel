ushort allocate_gdt_selector(void);
int free_gdt_selector(ushort sel);

ushort create_ldt(int size);
int destroy_ldt(ushort sel);

ushort allocate_ldt_selector(ushort ldt);
ushort allocate_some_ldt_selectors(ushort ldt, int count);
int free_ldt_selector(ushort ldt, ushort sel);
