#define XMS_err_not_implemented		0x80
#define XMS_err_vdisk_found	   		0x81
#define XMS_err_a20			   		0x82
#define XMS_err_general		   		0x8e
#define XMS_err_unrecoverable  		0x8f

#define XMS_err_hma_not_exist		0x90
#define XMS_err_hma_in_use	   		0x91
#define XMS_err_hma_min_size		0x92
#define XMS_err_hma_not_alloced		0x93

#define XMS_err_out_of_memory  		0xa0
#define XMS_err_out_of_handles 		0xa1
#define XMS_err_invalid_handle 		0xa2
#define XMS_err_sh_invalid	   		0xa3
#define XMS_err_so_invalid	   		0xa4
#define XMS_err_dh_invalid	   		0xa5
#define XMS_err_do_invalid	   		0xa6
#define XMS_err_len_invalid	   		0xa7
#define XMS_err_overlap		   		0xa8
#define XMS_err_parity		   		0xa9
#define XMS_err_emb_unlocked   		0xaa
#define XMS_err_emb_locked	   		0xab
#define XMS_err_lock_overflow  		0xac
#define XMS_err_lock_fail	   		0xad

#define XMS_err_umb_size_too_big	0xb0
#define XMS_err_no_umbs		   		0xb1
#define XMS_err_invalid_umb	   		0xb2

ushort allocExtendedMemory(ulong amount); /* amount is in kilobytes */
int freeExtendedMemory(ushort handle);
ushort reallocExtendedMemory(ushort handle, ulong newSize);
ulong lockExtendedMemory(ushort handle);
int unlockExtendedMemory(ushort handle);
int moveExtendedMemory(ulong len, ushort srcH, ulong srcO, ushort dstH, ulong dstO);

extern unsigned char XMS_error;
