#include "types.h"


#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__



#define KEY_NONE        0x00
#define KEY_ENTER       '\n'
#define KEY_TAB         '\t'
#define KEY_ESC         0x1B
#define KEY_BACKSPACE   0x08

#define KEY_CTRL        0x81
#define KEY_LSHIFT      0x82
#define KEY_RSHIFT      0x83
#define KEY_PRINTSCREEN 0x84
#define KEY_LALT        0x85
#define KEY_CAPSLOCK    0x86
#define KEY_F1          0x87
#define KEY_F2          0x88
#define KEY_F3          0x89
#define KEY_F4          0x8A
#define KEY_F5          0x8B
#define KEY_F6          0x8C
#define KEY_F7          0x8D
#define KEY_F8          0x8E
#define KEY_F9          0x8F
#define KEY_F10         0x90
#define KEY_NUMLOCK     0x91
#define KEY_SCROLLLOCK  0x92
#define KEY_HOME        0x93
#define KEY_UP          0x94
#define KEY_PAGEUP      0x95
#define KEY_LEFT        0x96
#define KEY_CENTER      0x97
#define KEY_RIGHT       0x98
#define KEY_END         0x99
#define KEY_DOWN        0x9A
#define KEY_PAGEDOWN    0x9B
#define KEY_INS         0x9C
#define KEY_DEL         0x9D
#define KEY_F11         0x9E
#define KEY_F12         0x9F
#define KEY_PAUSE       0xA0

/*flags*/
#define KBD_FLAG_DOWN_CODE 		0b10000000
#define KBD_FLAG_EXTENDED_CODE 		0b00100000
#define KBD_FLAG_UP_CODE 		0b00010000	
#define KBD_FLAG_PAUSE			0b00001000




BYTE kwrite_kbd_ctl_reg(BYTE data);

BYTE kread_kbd_status_reg();
BYTE kwrite_kbd_inpb(BYTE data);

BYTE kread_kbd_outb();
BOOL kis_kbd_outb_full();

BOOL kis_kbd_inpb_full();

volatile BOOL kwait_limit_kbd_inpb_empty();

volatile BOOL kwait_limit_kbd_outb_full();
volatile BOOL kwait_limit_kbd_ack();


BOOL kactivate_kbd();
BYTE kread_kbd_sc();
BOOL kactivate_kbd_a20_gate();

void kreboot_with_kbd();


BOOL kwrite_kbd_led(BOOL caps_lock, BOOL num_lock, BOOL scroll_lock);

void kupdate_kbd_status( BYTE scancode );
BOOL kis_alphabet_sc(BYTE scancode);

//11까지 아닌지 확인
BOOL kis_number_symbol_sc(BYTE scancode);

BOOL kis_numberpad_sc(BYTE scancode);


BOOL kis_combined_sc(BYTE scancode);

BOOL ktranslate_kbd_sc_ascii(BYTE scancode, BYTE *ascii, BYTE *status_flags);



#endif