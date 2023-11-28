/**
 * nm_interface.h
 **/
#include "sectorlist.h"
#include "singlelist.h"
#include "lpartition.h"
#include "ppartition.h"
#include "nandmanagerinterface.h"

enum cmd {
	SUSPEND,
	RESUME,
};

typedef struct __nm_lpt NM_lpt;
typedef struct __nm_ppt NM_ppt;

struct __nm_lpt {
	struct singlelist list;
	LPartition *pt;
	int context;
	int refcnt;
};

struct __nm_ppt {
	struct singlelist list;
	PPartition *pt;
	int refcnt;
};

int NM_ptOpen(int handler, const char *name, int mode);
int NM_ptRead(int context, SectorList* bl);
int NM_ptWrite(int context, SectorList* bl);
int NM_ptIoctrl(int context, int cmd, int args);
int NM_ptErase(int context);
int NM_ptClose(int context);

int NM_DirectRead(NM_ppt *ppt, int pageid, int off_t, int bytes, void *data);
int NM_DirectWrite(NM_ppt *ppt, int pageid, int off_t, int bytes, void *data);
int NM_DirectErase(NM_ppt *ppt, int blockid);
int NM_DirectIsBadBlock(NM_ppt *ppt, int blockid);
int NM_DirectMarkBadBlock(NM_ppt *ppt, int blockid);
int NM_UpdateErrorPartition(NM_ppt *ppt);

NM_lpt* NM_getPartition(int handler);
NM_ppt* NM_getPPartition(int handler);
void NM_startNotify(int handler, void (*start)(int), int prdata);
int NM_PrepareNewFlash(int handler);
int NM_CheckUsedFlash(int handler);
int NM_EraseFlash(int handler, int force);
void NM_regPtInstallFn(int handler, int data);
int NM_ptInstall(int handler, char *ptname);
int NM_open(void);
void NM_close(int handler);
