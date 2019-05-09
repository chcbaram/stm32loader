/*
 * hw_def.h
 *
 *  Created on: 2018. 6. 1.
 *      Author: HanCheol Cho
 */

#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_


#include "def.h"


#define _USE_HW_DXL
#define _USE_HW_DXLPORT
#define _USE_HW_DXLCMD



#define _USE_HW_CMDIF_DXLCMD



#define _HW_DEF_CMDIF_LIST_MAX                32

#define _HW_DEF_DXLCMD_MAX_NODE               128
#define _HW_DEF_DXLCMD_MAX_NODE_BUFFER_SIZE   128
#define _HW_DEF_DXLCMD_BUFFER_MAX             2048
#define _HW_DEF_DXL_MAX_BUFFER                (2048+10)




#endif /* SRC_HW_HW_DEF_H_ */
