#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char BYTE, BOOL;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned long QWORD;

#define TRUE 1
#define FALSE 0
#define NULL ((void*)0)

#define MONITORWIDTH 80
#define MONITORHEIGHT 24

#pragma pack(push, 1)

struct kcharacter {
	BYTE character;
	BYTE attribute;
};

#pragma pack( pop )

#endif /*__TYPES_H__*/