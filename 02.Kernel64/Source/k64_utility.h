#include "types.h"

#ifndef __K64_UTILITY_H__
#define __K64_UTILITY_H__


void kprint(int x, int y, const char *str);
//dest 항상 MONITORWIDTHbytes 이상이어야 한다. 
void kprint_form(int x, int y, const char *str);
void kprint_mark(int y, BOOL mark);

#endif /*__K64_UTILITY_H__*/	