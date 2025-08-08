#include "types.h"
#include "keyboard.h"
#include "k64_utility.h"

void kprint(int x, int y, const char *str);
void kprint_form(int x, int y, const char *str);
void kprint_mark(int y, BOOL mark);



void main()
{	
	BOOL tmp = FALSE;

	/*keyboard var*/
	BYTE scancode;
	BYTE ascii[2] = {0,};
	BYTE flags;
	int monitor_pos = 0;

	kprint_form(0, 11, "Start 64bits C kernel");
	kprint_mark(11, TRUE);

	
	kprint_form(0, 12, "Activate Keyboard");
	tmp = kactivate_kbd();
	kprint_mark(12, tmp);
	if (!tmp)
		goto INFINITE_LOOP;
	

	/*
	todo:
		down key일 때 처리하면, 한번에 공백이 많이 생김 이유 알기
		쉬프트 엔터 등 키 이상한 문자 출력 안되게 하기
	*/
	for (;;) {
		scancode = kread_kbd_sc();

		tmp = ktranslate_kbd_sc_ascii(scancode, ascii, &flags);
		if (tmp  && !(flags & KBD_FLAG_EXTENDED_CODE)) {
			// kprint(monitor_pos % MONITORWIDTH, monitor_pos / MONITORWIDTH, ascii);
			if (flags & KBD_FLAG_UP_CODE) {
				kprint(monitor_pos, 13, ascii);
				monitor_pos++;
			}
			// else if (flags & KBD_FLAG_UP_CODE)
			// 	monitor_pos++;
			
		}
	}

	INFINITE_LOOP:
	while(1);
	
}



