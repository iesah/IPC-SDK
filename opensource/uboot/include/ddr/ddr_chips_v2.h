#ifndef __DDR_CHIPS_V2_H__
#define __DDR_CHIPS_V2_H__

#include  "chips-v2/ddr_chip.h"

#include "chips-v2/LPDDR2_W97BV6MK.h"
#include "chips-v2/LPDDR3_W63AH6NKB-BI.h"
#include "chips-v2/LPDDR2_M54D5121632A.h"


#include "chips-v2/LPDDR3_W63CH2MBVABE.h"
#include "chips-v2/LPDDR3_MT52L256M32D1PF.h"

#ifdef CONFIG_DDR2_M14D5121632A
#include "chips-v2/DDR2_M14D5121632A.h"
#endif
#ifdef CONFIG_DDR3_W631GU6NG
#include "chips-v2/DDR3_W631GU6NG.h"
#endif
#endif
