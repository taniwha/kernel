/* dos emulation layer
 */
#include "types.h"
#include "kmalloc.h"
#include "rmcb.h"
#include "dos.h"

typedef struct {
	FarPointer next		__attribute__((packed));
	ushort flags		__attribute__((packed));
	ushort strategy		__attribute__((packed));
	ushort interupt		__attribute__((packed));
	union {
		uchar num_units	__attribute__((packed));	
		char name[8]	__attribute__((packed));
	} n;
} DeviceHeader;

extern struct SysVars {
	ushort			int215e01cx					__attribute__((packed));
	ushort			lruFCBcache					__attribute__((packed));
	ushort			lruFCBopens					__attribute__((packed));
	FarPointer		OEMfunction					__attribute__((packed));
	ushort			int21DOSds					__attribute__((packed));
	ushort			shareRetryCount				__attribute__((packed));
	ushort			shareRetryDelay				__attribute__((packed));
	FarPointer		currentDiskBuffer			__attribute__((packed));
	ushort			unreadConInput				__attribute__((packed));
	ushort			firstMCBsegment				__attribute__((packed));
	FarPointer		firstDriveParamBlock		__attribute__((packed));
	FarPointer		firstSystemFileTable		__attribute__((packed));
	FarPointer		clockDeviceHeader			__attribute__((packed));
	FarPointer		conDeviceHeader				__attribute__((packed));
	ushort			maxBPS						__attribute__((packed));
	FarPointer		diskBufferInfo				__attribute__((packed));
	FarPointer		currentDirectoryStructures	__attribute__((packed));
	FarPointer		systemFCBtables				__attribute__((packed));
	ushort			numProtFCBs					__attribute__((packed));
	uchar			numBlockDevices				__attribute__((packed));
	uchar			numDriveLetters				__attribute__((packed));
	DeviceHeader	NULLdeviceHeader			__attribute__((packed));
	uchar			numJoinedDrives				__attribute__((packed));
	ushort			specialNames				__attribute__((packed));
	FarPointer		SETVERlist					__attribute__((packed));
	ushort			A20fixOffset				__attribute__((packed));
	ushort			hmaPSP						__attribute__((packed));
	ushort			numBuffers					__attribute__((packed));
	ushort			numLookBuffers				__attribute__((packed));
	uchar			bootDrive					__attribute__((packed));
	uchar			use386						__attribute__((packed));
	ushort			emsSize						__attribute__((packed));
} sysvars;
asm(".set _sysvars, 0x100e"); /* this is where open dos puts them */

ushort dos_current_psp;

/* Save the contents of the original real mode interrupt vector table.  This
 * is for future versions that might support multiple dos boxes so that each
 * box can start out with the same interrupt table.
 */
FarPointer original_ivects[256];
extern FarPointer ivects[256];

static void dos_int20(RMCB_Regs *regs)
{
/*	terminate_program(regs,0,0);*/
}

static void unsupported(RMCB_Regs *regs)
{
	dos_error(regs,1);
}

static void dos_api(RMCB_Regs *regs)
{
	uchar ah=(regs->eax>>8)&0xff;
	uchar al=(regs->eax)&0xff;

	regs->eflags&=~1;	/* clear carry flag */
	asm("sti"); /* re-enable interrupts */
	switch (ah) {
	case 0x00:
/*		terminate_program(regs,0,0);
		break;
	case 0x01:	case 0x02:	case 0x03:	case 0x04:
	case 0x05:	case 0x06:	case 0x07:	case 0x08:
	case 0x09:	case 0x0a:	case 0x0b:	case 0x0c:
		really_wierd_guff(ah,regs);
		break;
	case 0x0d:
		disk_reset();
		break;
	case 0x0e:
		regs->eax&=~0xff;
		regs->eax|=select_default_drive(regs->edx&0xff)&0xff;*/
		break;
	case 0x0f:		/* fcb open */
	case 0x10:		/* fcb close */
	case 0x11:		/* fcb find first */
	case 0x12:		/* fcb find next */
	case 0x13:		/* fcb delete */
	case 0x14:		/* fcb sequential read */
	case 0x15: 		/* fcb sequential write */
	case 0x16:		/* fcb create or truncate */
	case 0x17:		/* fcb rename */
	case 0x21:		/* fcb read random record */
	case 0x22:		/* fcb write random record */
	case 0x23:		/* fcb get file size */
	case 0x24:		/* fcb set random record number (seek?) */
	case 0x27:		/* fcb read random block */
	case 0x28:		/* fcb write random block */
	case 0x29:		/* fcb parse filename */
		unsupported(regs);
		break;
	case 0x18:		/* null function */
	case 0x1d:		/* null function */
	case 0x1e:		/* null function */
	case 0x20:		/* null function */
	case 0x61:		/* null function */
	case 0x6b:		/* null function */
		regs->eax&=!0xff;
		break;
	case 0x19:
/*		regs->eax&=~0xff;
		regs->eax|=get_default_drive();
		break;
	case 0x1a:
		set_dta(regs->ds,regs->edx&0xffff);
		break;
	case 0x1b:
		regs->edx&=~0xff;
		regs->edx|=get_default_drive();*/
		/* fall through */
/*	case 0x1c:
		get_disk_alloc_info(regs);
		break;*/
	case 0x25:
		asm("cli");
		ivects[al].offset=regs->edx&0xffff;
		ivects[al].segment=regs->ds;
		break;
/*	case 0x26:
		create_new_psp(regs);
		break;
	case 0x2a:
		get_date(regs);
		break;
	case 0x2b:
		set_date(regs);
		break;
	case 0x2c:
		get_time(regs);
		break;
	case 0x2d:
		set_time(regs);
		break;
	case 0x2e:
		set_verify(al);
		break;
	case 0x2f:
		get_dta(&regs->ds,(ushort*)&regs->edx);
		break;*/
	case 0x30:
		regs->eax&=!0xffff;
		regs->eax|=0x0040;
		regs->ebx&=0xffff00ff;	/* not quite true, I'm not IBM :) */
		break;
/*	case 0x31:
		terminate_program(regs,1,1);
		break;
	case 0x1f:
		regs->edx&=~0xff;
		regs->edx|=get_default_drive();*/
		/* fall through */
	case 0x32:
/*		get_dpb(regs);*/
		break;
	case 0x33:		/* misc */
		break;
	case 0x34:		/* get indos flag */
		regs->es=0;
		regs->ebx&=~0xffff;
		break;
	case 0x35:
		asm("cli");
		regs->es=ivects[al].segment;
		regs->ebx&=~0xffff;
		regs->ebx|=ivects[al].offset;
		break;
	case 0x36:
/*		get_free_disk_space(regs);*/
		break;
	case 0x37:		/* switch char */
	case 0x38:		/* contry specific info */
		unsupported(regs);
		break;
	case 0x39:		/* mkdir */
	case 0x3a:		/* rmdir */
	case 0x3b:		/* chdir */
	case 0x3c:		/* creat */
	case 0x3d:		/* open */
	case 0x3e:		/* close */
	case 0x3f:		/* read */
	case 0x40:		/* write */
	case 0x41:		/* unlink */
	case 0x42:		/* lseek */
	case 0x43:		/* chmod */
	case 0x44:		/* ioctl */
	case 0x45:		/* dup */
	case 0x46:		/* dup2 */
	case 0x47:		/* cwd */
		break;
	/* Memory routines */
	case 0x48:		/* allocate memory */
		dos_allocate_memory(regs);
		break;
	case 0x49:		/* free memory */
		dos_free_memory(regs);
		break;
	case 0x4a:		/* resize memory */
		dos_resize_memory(regs);
		break;
	case 0x58:		/* memory allocation strategy */
		dos_memory_allocation_strategy(regs);
		break;

	case 0x4b:		/* exec */
		break;
	case 0x4c:		/* exit */
/*		terminate_program(regs,1,0);*/
		break;
	case 0x4d:		/* get return code */
	case 0x4e:		/* find first */
	case 0x4f:		/* find next */
	case 0x50:		/* set psp */
	case 0x51:		/* get psp */
	case 0x62:		/* get psp */
	case 0x52:		/* get list of lists */
	case 0x53:		/* xlat BPB to DPB */
	case 0x54:		/* get verify flag */
	case 0x55:		/* create child psp */
	case 0x56:		/* rename */
	case 0x57:		/* file extended attributes */
	case 0x59:		/* error info */
	case 0x5a:		/* create temporary file */
	case 0x5b:		/* create new file */
	case 0x5c:		/* "flock" */
		break;
	case 0x5d:		/* server function call */
	case 0x5e:		/* network guff */
	case 0x5f:		/* network guff */
		unsupported(regs);
		break;
	case 0x60:		/* truename */
	case 0x63:		/* intl8n */
	case 0x64:		/* set device driver lookahead flag */
	case 0x65:		/* get extended country info */
	case 0x66:		/* code pages */
	case 0x67:		/* set handle count */
	case 0x68:		/* fflush */
	case 0x69:		/* disk serial no */
	case 0x6a:		/* commit file */
	case 0x6c:		/* extended open/create */
		break;
	case 0x6d:		/* ROM dos stuff */
	case 0x6e:		/* ROM dos stuff */
	case 0x6f:		/* ROM dos stuff */
		unsupported(regs);
		break;
	case 0x71:		/* long file name support */
/*		lfn(regs);*/
		break;
	case 0x73:		/* W95 extensions */
		/* fat 32 etc */
		break;
	/* OEM guff */
	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfb:
	case 0xfc:
	case 0xfd:
	case 0xfe:
	case 0xff:
		unsupported(regs);
		break;
	default:
		unsupported(regs);
		break;
	}
}

static void __attribute__((constructor)) init_dos(void)
{
	asm(".set _dos_first_mcb,%0;.globl _dos_first_mcb": :"m"(sysvars.firstMCBsegment));
	set_rmcb_int(0x20,dos_int20);
	set_rmcb_int(0x21,dos_api);
	memcpy(original_ivects,ivects,sizeof(original_ivects));
	memset(&sysvars,0,sizeof(sysvars));
}
