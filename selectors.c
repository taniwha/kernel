#include "types.h"
#include "selectors.h"
#include "cpu.h"
#include "__main.h"
#include "kmalloc.h"

static Descriptor *gdt_descriptors;
static ulong *gdt_free_map;

static struct LDT {
	struct LDT *next;
	ushort size;
	ushort selector;
	ulong *free_map;
	Descriptor *descriptors;
} *ldt_data, *ldt_data_free_list;

static int _findFreeSel(ulong *map, ushort start, ushort len)
{
	int i;
	char found;
	ulong sels;
	ulong ind;

	if ((i=start%32)) {
		if ((sels=map[start/32]&(((ulong)-1)<<i))) {
			ulong sel;
			asm ("bsfl %k1,%k0":"=g"(sel):"g"(sels));
			return start/32+sel;
		}
		start+=32-i;
	}
	i = (len/32)-(start/32);
	asm(""
		"repz;"
		"scasl;"
		"setnz	%b0;"
		"subl	%k2,%k1;"
		"shrl	$2,%k1;"
		"decl	%k1;"
		""
		:"=r"(found),"=D"(ind),"+c"(i)
		:"m"(map),"1"(map+start/32),"a"(0)
		:"cc"
	);
	if (found) {
		ushort sel;
		asm ("bsfl %k1,%k0":"=g"(sel):"g"(map[ind]));
		return ind*32+sel;
	} else if ((i=len%32)) {
		ind=len/32;
		if ((sels=map[ind]&(((ulong)-1)>>(32-i)))) {
			ulong sel;
			asm ("bsfl %k1,%k0":"=g"(sel):"g"(sels));
			return ind*32+sel;
		}
	}
	return -1;
}

ushort allocate_gdt_selector(void)
{
	int sel;

	if (!gdt_descriptors) {
		if (!(gdt_descriptors=(Descriptor*)kmalloc(0x10000))) {
			return 0;
		}
		if (!(gdt_free_map=(ulong*)kmalloc(1024))) {
			kfree(gdt_descriptors);
			return 0;
		}
		memset(gdt_descriptors,0,0x10000);
		memset(gdt_free_map,0xff,1024);
		memcpy(gdt_descriptors,gdt,sizeof(Descriptor)*NUM_DESCRIPTORS);
		gdt_descriptors[0].pseudo.limit=0xffff;
		gdt_descriptors[0].pseudo.base=(ulong)gdt_descriptors;
		for (sel=0; sel<NUM_DESCRIPTORS; sel++) {
			reset_bit(gdt_free_map,sel);
		}
	}
	if ((sel=_findFreeSel(gdt_free_map,0,0x2000))<0) return 0;
	reset_bit(gdt_free_map,sel);
	return sel*8;
}

int free_gdt_selector(ushort sel)
{
	if ((sel&4) || set_bit(gdt_free_map,sel/8)) return 0;
	gdt_descriptors[sel/8]._pseudo.limit=gdt_descriptors[sel/8]._pseudo.limit=0;
	return 1;
}

static struct LDT *new_ldt_data(void)
{
	struct LDT *ldt;

	if (!ldt_data_free_list) {
		if (!(ldt_data_free_list=(struct LDT*)kmalloc(sizeof(struct LDT)*4))) {
			return 0;
		}
		ldt_data_free_list[3].next=0;
		ldt_data_free_list[2].next=&ldt_data_free_list[3];
		ldt_data_free_list[1].next=&ldt_data_free_list[2];
		ldt_data_free_list[0].next=&ldt_data_free_list[1];
	}
	ldt=ldt_data_free_list;
	ldt_data_free_list=ldt_data_free_list->next;
	return ldt;
}

static void free_ldt_data(struct LDT *ldt)
{
	ldt->next=ldt_data_free_list;
	ldt_data_free_list=ldt;
}

static struct LDT **find_ldt(ushort ldt)
{
	struct LDT **ldt_d=&ldt_data,*p;

	if (ldt&7) return 0;

	while (*ldt_d && (*ldt_d)->selector!=ldt) {
		ldt_d=&(*ldt_d)->next;
	}
	if (!ldt_d) return 0;
	p=*ldt_d;
	*ldt_d=(*ldt_d)->next;
	p->next=ldt_data;
	ldt_data=p;
	return &ldt_data;
}

ushort create_ldt(int size)
{
	struct LDT *ldt;

	if (size<=0x2000) {
		if ((ldt=new_ldt_data())) {
			if ((ldt->selector=allocate_gdt_selector())) {
				const int descSize=size*sizeof(Descriptor);
				if ((ldt->descriptors=(Descriptor*)kmalloc(descSize))) {
					const int mapSize=((size+31)/32)*sizeof(ulong);/* hmm, magic numbers */
					if ((ldt->free_map=(ulong*)kmalloc(mapSize))) {
						ldt->size=size;
						ldt->next=ldt_data;
						ldt_data=ldt;
						memset(ldt->descriptors,0,descSize);
						memset(ldt->free_map,0xff,mapSize);
						gdt_descriptors[ldt->selector/8].segment.limit0=descSize;
						gdt_descriptors[ldt->selector/8].segment.base0=
							(ulong)ldt->descriptors;
						gdt_descriptors[ldt->selector/8].segment.type=dt_ldt;
						gdt_descriptors[ldt->selector/8].segment.dpl=0;
						gdt_descriptors[ldt->selector/8].segment.present=1;
						gdt_descriptors[ldt->selector/8].segment.limit1=0;
						gdt_descriptors[ldt->selector/8].segment.avail=0;
						gdt_descriptors[ldt->selector/8].segment.use32=1;
						gdt_descriptors[ldt->selector/8].segment.gran=0;
						gdt_descriptors[ldt->selector/8].segment.base1=
							((ulong)ldt->descriptors)>>24;
						return ldt->selector;
					}
					kfree(ldt->descriptors);
				}
				free_gdt_selector(ldt->selector);
			}
			free_ldt_data(ldt);
		}
	}
	return 0;
}

int destroy_ldt(ushort ldt)
{
	struct LDT **ldt_d,*p;

	if (!(ldt_d=find_ldt(ldt))) return 0;
	if (!free_gdt_selector((*ldt_d)->selector)) return 0;
	p=*ldt_d;
	(*ldt_d)=(*ldt_d)->next;
	
	kfree(p->descriptors);
	kfree(p->free_map);
	free_ldt_data(p);
	return 1;
}

ushort allocate_ldt_selector(ushort ldt)
{
	int sel;
	struct LDT **ldt_d;
	if (!(ldt_d=find_ldt(ldt))) return 0;
	
	if ((sel=_findFreeSel((*ldt_d)->free_map,0,(*ldt_d)->size))<0) return 0;
	reset_bit((*ldt_d)->free_map,sel);
	return sel*8+4;
}

ushort allocate_some_ldt_selectors(ushort ldt, int count)
{
	int sel=0,i;
	struct LDT **ldt_d;
	if (!(ldt_d=find_ldt(ldt))) return 0;
	if (count>(*ldt_d)->size) return 0;

	while (sel<(*ldt_d)->size) {
		if ((sel=_findFreeSel((*ldt_d)->free_map,sel,(*ldt_d)->size))<0) return 0;
		for (i=sel; i<sel+count && test_bit((*ldt_d)->free_map,i); i++);
		if (i==sel+count) {
			for (i=sel; i<sel+count; i++) {
				reset_bit((*ldt_d)->free_map,i);
			}
			return sel*8+4;
		}
		sel+=i;
	}
	return 0;
}

int free_ldt_selector(ushort ldt, ushort sel)
{
	struct LDT **ldt_d;
	if (!(ldt_d=find_ldt(ldt))) return 0;
	if (!(sel&4)) return 0;
	if (set_bit((*ldt_d)->free_map,sel/8)) return 0;
	(*ldt_d)->descriptors[sel/8]._pseudo.limit=
		(*ldt_d)->descriptors[sel/8]._pseudo.limit=0;
	return 1;
}
