#include "types.h"


#ifndef __MODESWITCH_H__
#define __MODESWITCH_H__

void kread_cpuid(DWORD eax, DWORD *peax, DWORD *ebx, DWORD *ecx, DWORD *edx);
void kswitch_exe_k64(void);


#endif /*__MODESWITCH_H__*/