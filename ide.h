#define IDE_DATA(base)			((base)+0)	/* read/write,16 bits */
#define IDE_ERROR(base)			((base)+1)	/* read only */
#define IDE_WPC(base)			((base)+1)	/* write only */
#define IDE_SECTOR_COUNT(base)	((base)+2)	/* read/write */
#define IDE_SECTOR_NUMBER(base)	((base)+3)	/* read/write */
#define IDE_CYLINDER_LOW(base)	((base)+4)	/* read/write */
#define IDE_CYLINDER_HIGH(base)	((base)+5)	/* read/write */
#define IDE_DRIVE_HEAD(base)	((base)+6)	/* read/write */
#define IDE_STATUS(base)		((base)+7)	/* read only */
#define IDE_COMMAND(base)		((base)+7)	/* write only */

#define IDE_LBA_COUNT(base)		((base)+2)	/* read/write */
#define IDE_LBA_LOW(base)		((base)+3)	/* read/write LBA bits 0-7 */
#define IDE_LBA_MID(base)		((base)+4)	/* read/write LBA bits 8-15 */
#define IDE_LBA_HIGH(base)		((base)+5)	/* read/write LBA bits 16-23 */

#define IDE_CONTROL				0x3f6		/* bit 3 enables head bit 3 */

int ide_read_sectors(int drive, ulong sector, int count, void *buff);
int ide_write_sectors(int drive, ulong sector, int count, void *buff);
int ide_num_drives(void);
