#include "k64_utility.h"


void kprint(int x, int y, const char *str)
{	
	if (str == NULL)
		return;

	struct kcharacter *screen = (struct kcharacter*)0xb8000;//!!
	int i = 0;

	screen += (y * MONITORWIDTH) + x;

	for (;str[i] != '\0'; i++) {
		screen[i].character = str[i];	
	}

}

//dest 항상 MONITORWIDTHbytes 이상이어야 한다. 
void kprint_form(int x, int y, const char *str)
{
	kprint(x, y, "[     ] ");
	kprint(x + 8, y, str);
}

void kprint_mark(int y, BOOL mark)
{
	if (mark == TRUE)
		kprint(1, y, "PASS");
	else
		kprint(1, y, "ERROR");
}