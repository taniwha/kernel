#include "types.h"
#include "lfsys.h"
#include "bc_fsys.h"
#include "screen.h"
#include "kmalloc.h"

static int num_volumes;
static BC_Volume *volumes;

static void __attribute__((constructor)) bc_init(void)
{
	int num_partitions=lfsys_num_partitions();
	int partition;

	for (partition=0; partition<num_partitions; partition++) {
		if (lfsys_get_partition(partition)->type==KERNEL_FS) {
			num_volumes++;
		}
	}
	if (!num_volumes) return;
	volumes=(BC_Volume*)kmalloc(num_volumes*sizeof(BC_Volume));
	if (!volumes) return;
	kprintf("found %d kernel volume%s\n",num_volumes,num_volumes==1?"":"s");
	num_volumes=0;
	for (partition=0; partition<num_partitions; partition++) {
		const Partition *part=lfsys_get_partition(partition);
		if (part->type==KERNEL_FS) {
			BC_BootRecord br;
			BC_Volume *vol=&volumes[num_volumes];
			vol->partition=partition;
			vol->size=part->num_sectors;
			lfsys_read_sectors(partition,0,1,&br);
			if (br.kmagic==KMAGIC && br.magic==BIOS_MAGIC) {
				vol->root=br.root_sector;
				vol->root_size=br.root_size;
				vol->secmap=br.secmap_sector;
				vol->secmap_size=br.secmap_size;
				kprintf("vol:%d part:%d size:%d root:%d rsiz:%d smap:%d smsz:%d\n",
					num_volumes,
					vol->partition,
					vol->size,
					vol->root,
					vol->root_size,
					vol->secmap,
					vol->secmap_size
				);
				if (vol->size>vol->secmap_size*512*8) {
					kprintf("\t%d sectors unaccessable\n",
						vol->size-vol->secmap_size*512*8);
				}
			} else {
				vol->root=vol->root_size=vol->secmap=vol->secmap_size=0;
				kprintf("vol:%d unformatted?\n",num_volumes);
			}
			num_volumes++;
		}
	}
}
