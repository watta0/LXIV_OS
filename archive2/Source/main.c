#include "types.h"
#include "page.h"
#include "mode_switch.h"


void kprint( int x, int y, const char *str);
void kprint_form(int x, int y, const char *str);
void kprint_mark(int y, BOOL mark);

BOOL kinit_k64_area(void);
BOOL kis_memory_enough(void);
void kcopy_k64_2mb(void);

void main(void)
{
	BOOL tmp;
	DWORD eax, ebx, ecx, edx;
	char vendor_str[13] = {0,};

	kprint_form(0, 3, "Start 32bits C kernel");
	kprint_mark(3, TRUE);

	tmp = kis_memory_enough();
	kprint_form(0, 4, "Minimum Memory Size Check");
	kprint_mark(4, tmp);
	if (!tmp)
		goto infinite_loop;

	//IA-32e mode kernel area init
	tmp = kinit_k64_area();
	kprint_form(0, 5, "IA-32e Kernel Area initialization");
	kprint_mark(5, tmp);
	if (!tmp)
		goto infinite_loop;
	
	//IA-32e mode page tables init
	kprint_form(0, 6, "IA-32e Page Tables Initialize");
	kInitializePageTables();
	kprint_mark(6, TRUE);

	//processor vendor string
	kprint_form(0, 7, "Processor Vendor String : ");
	kread_cpuid(0x00, &eax,&ebx, &ecx, &edx);
	*(DWORD*)vendor_str = ebx;
	*((DWORD*)vendor_str + 1) = edx;
	*((DWORD*)vendor_str + 2) = ecx;
	kprint(34, 7, vendor_str);
	kprint_mark(7, TRUE);
		
	//64bits(IA-32e mode) support check 
	kprint_form(0, 8, "64bits Mode Support check");
	kread_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
	//tmp =  edx & (1 << 29); // tmp는  BOOL이고, edx는 DWORD이다. 아마 edx가 BOOL(unsigned char)로 바뀌어서, 연산이 잘못되는 것 같다. (항상 FALSE가 나옴)
	DWORD fucking_tmp = (edx >> 29) & 1;	//edx & (1 << 29) 안됨, 되는데 뭔가 이상함 
        kprint_mark(8, fucking_tmp);
	if (!fucking_tmp)
		goto infinite_loop;
	
	//move kernel64 image to 2MB area
	//bootloader가 disk에서 k32, k64순서대로 이미지를 복사해 온다.(0x10000)
	//bootloader 앞부분 totalsectorcount 와 k32sectorcount 를 사용하여 k64img만 2MB 영역으로 옮긴다. 
	kprint_form(0, 9, "Translocate 64bits kernel");
	kcopy_k64_2mb();
	kprint_mark(9, TRUE);

	kswitch_exe_k64();


		

	infinite_loop:
	while(1);
}




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

BOOL kinit_k64_area(void)
{
	DWORD *current_addr = (DWORD*)0x100000; //1MB

	while((DWORD)current_addr < 0x600000) {
		*current_addr = 0x00;
		if (*current_addr != 0x00)
			return FALSE;
		
		current_addr++;
	}
	return TRUE;	
}



BOOL kis_memory_enough(void)
{
	DWORD *current_addr = (DWORD*)0x100000;
	int i;
	const DWORD dummy1 = 0x1234;
	const DWORD dummy2 = 0x5678;
	while (current_addr < (DWORD*)0x4000000) {
		current_addr[0] = dummy1;
		current_addr[1] = dummy2;

		if(current_addr[0] != dummy1 || current_addr[1] != dummy2)
			return FALSE;
		
		current_addr += 0x100000/sizeof(DWORD);
	}

	return TRUE;
}

void kcopy_k64_2mb(void)
{
	WORD total_sector_cnt =*((WORD*)0x7c05);
	WORD k32_sector_cnt	= *((WORD*)0x7c07);

	DWORD *src_addr = (DWORD*) (0x10000 + k32_sector_cnt * 512);
	DWORD *target_addr = (DWORD*)0x200000;

	DWORD size = 512 * (total_sector_cnt - k32_sector_cnt)  / sizeof(DWORD); 


	for (int i = 0; i < size; i++) {
		target_addr[i] = src_addr[i];
	}
}