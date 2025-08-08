#include "keyboard.h"
#include "assembly_utility.h"
#include "k64_utility.h"

//헤더로 뺄지 고민. 어셈블리 유틸리티도
#define KBD_CTL_REG			0x64	//WRITE controll register
#define KBD_STATUS_REG 			0x64	//read keyboard status
#define KBD_INPB 			0x60	//transmit buffer
#define KBD_OUTB 			0x60	//receive buffer

/*STATUS REGISTER*/
#define KBD_STATUS_PARE 		128
#define KBD_STATUS_TIM 			64
#define KBD_STATUS_AUXB 		32
#define KBD_STATUS_KEYL 		16
#define KBD_STATUS_CD 			8
#define KBD_STATUS_SYSF 		4
#define KBD_STATUS_INPB 		2
#define KBD_STATUS_OUTB 		1

/*keyboard controll register command*/
#define KBD_CTL_ACTIVATE_KBD 		0XAE
#define KBD_CTL_DEACTIVATE_KBD 		0xAD
#define KBD_CTL_ACTIVATE_MOUSE 		0xA8
#define KBD_CTL_DEACTIVATE_MOUSE 	0xA7

#define KBD_CTL_READ_OUTPORT		0xD0
#define KBD_CTL_WRITE_OUTPORT		0xD1


/*keyboard output buffer command*/
#define KBD_OUTB_ACK 			0xFA

/*keyboard input buffer command*/
#define KBD_INPB_ACTIVATE_KBD 		0XF4
#define KBD_INPB_LED			0xED

/**/
#define KBD_WAIT_IO			0xFFFF
#define KBD_ACK_WAIT_CNT 		100
#define KBD_IGNORE_CNT_FOR_PAUSE 	2
#define KEY_MAPPINGTABLEMAXCOUNT    	89


struct kbd_key_map_entry {
	BYTE normal_code;
	BYTE combined_code;
};

struct kbd_manager {
	int ignore_cnt_for_pause;	//유일하게 0xe1으로 시작한다. 따라서 해당 코드 이후 2개의 scancode 는 무시한다. 
	int ext_code_cnt;		//0xe0로 시작 이후 2개의 scancode 
	BOOL extended_code;

	BOOL shift_on;
	BOOL caps_lock_on;
	BOOL num_lock_on;
	BOOL scroll_lock_on;
};



//caution: 인자로 넣은 함수의 성공을 보장하지 않음
#define UNTIL_LIMIT(mis_function_pointer)	for (QWORD until_limit_counter = 0; until_limit_counter < KBD_WAIT_IO ; until_limit_counter++)	if (mis_function_pointer())	return;		 

BYTE kwrite_kbd_ctl_reg(BYTE data)
{
	return ktransmit_kbd_byte(KBD_CTL_REG, data);
}

BYTE kread_kbd_status_reg()
{
	return kreceive_kbd_byte(KBD_STATUS_REG);
}		

BYTE kwrite_kbd_inpb(BYTE data)
{
	return ktransmit_kbd_byte(KBD_INPB, data);
}

BYTE kread_kbd_outb()
{	
	return kreceive_kbd_byte(KBD_OUTB);
}

BOOL kis_kbd_outb_full()
{
	if (kreceive_kbd_byte(KBD_STATUS_REG) & KBD_STATUS_OUTB)
		return TRUE;
	else
		return FALSE;
}

BOOL kis_kbd_inpb_full()
{
	if (kreceive_kbd_byte(KBD_STATUS_REG) & KBD_STATUS_INPB)
		return TRUE;
	else
		return FALSE;
}

volatile BOOL kwait_limit_kbd_inpb_empty()
{
	for (volatile int i = 0; i < KBD_WAIT_IO; i++) {
		if (kis_kbd_inpb_full() == FALSE)
			return TRUE;
	}

	return FALSE;
}


volatile BOOL kwait_limit_kbd_outb_full()
{
	for (volatile int i = 0; i < KBD_WAIT_IO; i++) {
		if (kis_kbd_outb_full())
			return TRUE;
	}

	return FALSE;
}

volatile BOOL kwait_limit_kbd_ack()
{
	for (volatile int i = 0; i < KBD_ACK_WAIT_CNT; i++) {
		kwait_limit_kbd_outb_full();

		if (kread_kbd_outb() == KBD_OUTB_ACK)
			return TRUE;
	}
	return FALSE;
}


BOOL kactivate_kbd()
{
	kwrite_kbd_ctl_reg(KBD_CTL_ACTIVATE_KBD);	//activate keyboard in controll register 
	
	if (kwait_limit_kbd_inpb_empty() == FALSE)
		return FALSE;

	kwrite_kbd_inpb(KBD_INPB_ACTIVATE_KBD);
		
	return kwait_limit_kbd_ack();
}

BYTE kread_kbd_sc()
{

	if (kwait_limit_kbd_outb_full() == FALSE)
		return FALSE;

	return kread_kbd_outb();
}

BOOL kactivate_kbd_a20_gate()
{
	kwrite_kbd_ctl_reg(KBD_CTL_READ_OUTPORT);

	if (kwait_limit_kbd_outb_full() == FALSE)
		return FALSE;
	
	//not output buffer. it's kbd controll register's output port
	BYTE outport = kread_kbd_outb();

	outport |= 0x02; //a20 gate activate bit

	if (kwait_limit_kbd_inpb_empty() == FALSE)
		return FALSE;
	
	kwrite_kbd_inpb(KBD_CTL_WRITE_OUTPORT);
	kwrite_kbd_inpb(outport);

}

void kreboot_with_kbd()
{
	kwrite_kbd_ctl_reg(KBD_CTL_READ_OUTPORT);

	if (kwait_limit_kbd_outb_full() == FALSE)
		return;
	
	//not output buffer. it's kbd controll register's output port
	BYTE outport = kread_kbd_outb();

	outport = 0x00; //a20 gate activate bit

	if (kwait_limit_kbd_inpb_empty() == FALSE)
		return;

	kwrite_kbd_inpb(KBD_CTL_WRITE_OUTPORT);
	kwrite_kbd_inpb(outport);
}

/*translate sc to ascii*/
static struct kbd_manager keyboard_manager;
static struct kbd_key_map_entry key_map[ KEY_MAPPINGTABLEMAXCOUNT ] =
{
    /*  0   */  {   KEY_NONE        ,   KEY_NONE        },
    /*  1   */  {   KEY_ESC         ,   KEY_ESC         },
    /*  2   */  {   '1'             ,   '!'             },
    /*  3   */  {   '2'             ,   '@'             },
    /*  4   */  {   '3'             ,   '#'             },
    /*  5   */  {   '4'             ,   '$'             },
    /*  6   */  {   '5'             ,   '%'             },
    /*  7   */  {   '6'             ,   '^'             },
    /*  8   */  {   '7'             ,   '&'             },
    /*  9   */  {   '8'             ,   '*'             },
    /*  10  */  {   '9'             ,   '('             },
    /*  11  */  {   '0'             ,   ')'             },
    /*  12  */  {   '-'             ,   '_'             },
    /*  13  */  {   '='             ,   '+'             },
    /*  14  */  {   KEY_BACKSPACE   ,   KEY_BACKSPACE   },
    /*  15  */  {   KEY_TAB         ,   KEY_TAB         },
    /*  16  */  {   'q'             ,   'Q'             },
    /*  17  */  {   'w'             ,   'W'             },
    /*  18  */  {   'e'             ,   'E'             },
    /*  19  */  {   'r'             ,   'R'             },
    /*  20  */  {   't'             ,   'T'             },
    /*  21  */  {   'y'             ,   'Y'             },
    /*  22  */  {   'u'             ,   'U'             },
    /*  23  */  {   'i'             ,   'I'             },
    /*  24  */  {   'o'             ,   'O'             },
    /*  25  */  {   'p'             ,   'P'             },
    /*  26  */  {   '['             ,   '{'             },
    /*  27  */  {   ']'             ,   '}'             },
    /*  28  */  {   '\n'            ,   '\n'            },
    /*  29  */  {   KEY_CTRL        ,   KEY_CTRL        },
    /*  30  */  {   'a'             ,   'A'             },
    /*  31  */  {   's'             ,   'S'             },
    /*  32  */  {   'd'             ,   'D'             },
    /*  33  */  {   'f'             ,   'F'             },
    /*  34  */  {   'g'             ,   'G'             },
    /*  35  */  {   'h'             ,   'H'             },
    /*  36  */  {   'j'             ,   'J'             },
    /*  37  */  {   'k'             ,   'K'             },
    /*  38  */  {   'l'             ,   'L'             },
    /*  39  */  {   ';'             ,   ':'             },
    /*  40  */  {   '\''            ,   '\"'            },
    /*  41  */  {   '`'             ,   '~'             },
    /*  42  */  {   KEY_LSHIFT      ,   KEY_LSHIFT      },
    /*  43  */  {   '\\'            ,   '|'             },
    /*  44  */  {   'z'             ,   'Z'             },
    /*  45  */  {   'x'             ,   'X'             },
    /*  46  */  {   'c'             ,   'C'             },
    /*  47  */  {   'v'             ,   'V'             },
    /*  48  */  {   'b'             ,   'B'             },
    /*  49  */  {   'n'             ,   'N'             },
    /*  50  */  {   'm'             ,   'M'             },
    /*  51  */  {   ','             ,   '<'             },
    /*  52  */  {   '.'             ,   '>'             },
    /*  53  */  {   '/'             ,   '?'             },
    /*  54  */  {   KEY_RSHIFT      ,   KEY_RSHIFT      },
    /*  55  */  {   '*'             ,   '*'             },
    /*  56  */  {   KEY_LALT        ,   KEY_LALT        },
    /*  57  */  {   ' '             ,   ' '             },
    /*  58  */  {   KEY_CAPSLOCK    ,   KEY_CAPSLOCK    },
    /*  59  */  {   KEY_F1          ,   KEY_F1          },
    /*  60  */  {   KEY_F2          ,   KEY_F2          },
    /*  61  */  {   KEY_F3          ,   KEY_F3          },
    /*  62  */  {   KEY_F4          ,   KEY_F4          },
    /*  63  */  {   KEY_F5          ,   KEY_F5          },
    /*  64  */  {   KEY_F6          ,   KEY_F6          },
    /*  65  */  {   KEY_F7          ,   KEY_F7          },
    /*  66  */  {   KEY_F8          ,   KEY_F8          },
    /*  67  */  {   KEY_F9          ,   KEY_F9          },
    /*  68  */  {   KEY_F10         ,   KEY_F10         },
    /*  69  */  {   KEY_NUMLOCK     ,   KEY_NUMLOCK     },
    /*  70  */  {   KEY_SCROLLLOCK  ,   KEY_SCROLLLOCK  },

    /*  71  */  {   KEY_HOME        ,   '7'             },
    /*  72  */  {   KEY_UP          ,   '8'             },
    /*  73  */  {   KEY_PAGEUP      ,   '9'             },
    /*  74  */  {   '-'             ,   '-'             },
    /*  75  */  {   KEY_LEFT        ,   '4'             },
    /*  76  */  {   KEY_CENTER      ,   '5'             },
    /*  77  */  {   KEY_RIGHT       ,   '6'             },
    /*  78  */  {   '+'             ,   '+'             },
    /*  79  */  {   KEY_END         ,   '1'             },
    /*  80  */  {   KEY_DOWN        ,   '2'             },
    /*  81  */  {   KEY_PAGEDOWN    ,   '3'             },
    /*  82  */  {   KEY_INS         ,   '0'             },
    /*  83  */  {   KEY_DEL         ,   '.'             },
    /*  84  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  85  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  86  */  {   KEY_NONE        ,   KEY_NONE        },
    /*  87  */  {   KEY_F11         ,   KEY_F11         },
    /*  88  */  {   KEY_F12         ,   KEY_F12         }
};


BOOL kwrite_kbd_led(BOOL caps_lock, BOOL num_lock, BOOL scroll_lock)
{
	if (kwait_limit_kbd_inpb_empty() == FALSE)
		return FALSE;
	kwrite_kbd_inpb(KBD_INPB_LED);
	if (kwait_limit_kbd_inpb_empty() == FALSE)
		return FALSE;

	if (kwait_limit_kbd_ack() == FALSE)
		return FALSE;
	
	kwrite_kbd_inpb((caps_lock << 2) | (num_lock << 1) | (scroll_lock << 0));
	if (kwait_limit_kbd_inpb_empty() == FALSE)
		return FALSE;

	return kwait_limit_kbd_ack();
}

void kupdate_kbd_status( BYTE scancode )
{
    BOOL is_down = (scancode >> 7) ^ 1;

    BYTE down_code = scancode & 0x7F;

    //extneded 코드(keyboard_manager에서 확인)이라면 리턴

    if((down_code == 42) || (down_code == 54)) {
        keyboard_manager.shift_on = is_down;
	return;
    }
    if (!is_down)
    	return;

    if(down_code == 58) {
        keyboard_manager.caps_lock_on ^= TRUE;
    } else if(down_code == 69) {
        keyboard_manager.num_lock_on ^= TRUE;
    } else if(down_code == 70) {
        keyboard_manager.scroll_lock_on ^= TRUE;
    }

    kwrite_kbd_led( keyboard_manager.caps_lock_on,
        		keyboard_manager.num_lock_on,
				keyboard_manager.scroll_lock_on);
}

BOOL kis_alphabet_sc(BYTE scancode)
{
	BYTE down_code = scancode & 0x7F;
	if ('a' <= key_map[down_code].normal_code && key_map[down_code].normal_code <= 'z')
		return TRUE;
	return FALSE;
}

//11까지 아닌지 확인
BOOL kis_number_symbol_sc(BYTE scancode)
{	
	BYTE down_code = scancode & 0x7F;
	if ((2 <= down_code) && (down_code <= 53) && !kis_alphabet_sc(scancode))
		return TRUE;
	return FALSE;
}

BOOL kis_numberpad_sc(BYTE scancode)
{
	BYTE down_code = scancode & 0x7F;
	if ((71 <= down_code) && (83 >= down_code))
		return TRUE;
	return FALSE;
}

BOOL kis_combined_sc(BYTE scancode) 
{
	BYTE down_code = scancode  & 0x7F;

	if (kis_alphabet_sc(scancode) && (keyboard_manager.shift_on ^ keyboard_manager.caps_lock_on))
		return TRUE;
	
	if (kis_number_symbol_sc(scancode) && keyboard_manager.shift_on)
		return TRUE;
	
	if (kis_numberpad_sc(scancode) && keyboard_manager.num_lock_on && (keyboard_manager.extended_code == FALSE))//!!
		return TRUE;
	
	return FALSE;
}

BOOL ktranslate_kbd_sc_ascii(BYTE scancode, BYTE *ascii, BYTE *status_flags)
{
	BOOL is_combined;
	BOOL is_down = (scancode >> 7) ^ 1;//중요: scancode & 0x80 으로 한다면 작동 안함. 왜그런지 모르겠음 타입도 같은데...
	BYTE down_code = scancode & 0x7F;


	*status_flags = 0;

	if (is_down)
		*status_flags |=  KBD_FLAG_DOWN_CODE;
	else
		*status_flags |= KBD_FLAG_UP_CODE;

	if (keyboard_manager.ignore_cnt_for_pause > 0) {
		//pause키는 업 코드가 존재하지 않음 3개가 모두 다운코드
		*status_flags = 0; 
		keyboard_manager.ignore_cnt_for_pause--;
		return FALSE;
	}

	if (scancode == 0xE1) {
		keyboard_manager.ignore_cnt_for_pause = KBD_IGNORE_CNT_FOR_PAUSE;

		*status_flags = 0;
		*status_flags |= KBD_FLAG_DOWN_CODE | KBD_FLAG_EXTENDED_CODE | KBD_FLAG_PAUSE;

		return TRUE;
	}

	if (scancode == 0xE0) {
		keyboard_manager.extended_code = TRUE;
		keyboard_manager.ext_code_cnt = 2;

		*status_flags = 0;	//0xe0에서는 처리 못함 다음 다운코드를 봐야함
		*status_flags |= KBD_FLAG_EXTENDED_CODE;
		
		return FALSE;
	}
	
	if (keyboard_manager.ext_code_cnt > 0) {
		*status_flags |= KBD_FLAG_EXTENDED_CODE;
		keyboard_manager.ext_code_cnt--;
		*ascii = scancode;
		if (keyboard_manager.ext_code_cnt == 1) {
			return TRUE;
		} else {
			return FALSE;
		}
	}
	//		keyboard_manager.extended_code = FALSE;

	is_combined = kis_combined_sc(scancode);
	
	if (is_combined)
		*ascii = key_map[down_code].combined_code;
	else
		*ascii = key_map[down_code].normal_code;
	
	
	
	kupdate_kbd_status(scancode);
	
	return TRUE;

}
