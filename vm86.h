
typedef struct {
	long vmint;
	long vmflags;
	long vmesp;
	long vmss;
	long vmes;
	long vmds;
	long vmfs;
	long vmgs;
	long vmebp;
	long vmebx;
} _VMBlock;

typedef struct {
	long savesegs;
	long vmint;
	long vmflags;
	long vmesp;
	long vmss;
	long vmes;
	long vmds;
	long vmfs;
	long vmgs;
	long vmebp;
	long vmebx;
	long vmedx;
	long vmecx;
	long vmeax;
	long vmedi;
	long vmesi;
} VMBlock;

void vm86(VMBlock *vmRegisters);
