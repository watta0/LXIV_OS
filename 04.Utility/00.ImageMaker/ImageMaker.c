#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define BYTE_OF_SECTOR 512

int align_sector(int fd, int src_size);
void insert_sector_info(int bootld_fd, int k32_sector_cnt, int k64_sector_cnt);
int copy_sectors(int src_fd, int trgt_fd);

int main(int argc, char **argv)
{
	int src_fd, trgt_fd;
	int bootld_sector_cnt;
	int k32_sector_cnt;
	int k64_sector_cnt;
	int src_size;

	if (argc < 4) {
		fprintf(stderr, "[ERROR]  ImageMaker bootloaderFile kernelFile\nex) ImageMaker BootLoader.bin Kernel32.bin\n");
		exit(-1);	
	}

	//create Disk.img 
	trgt_fd = open("Disk.img", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
	if (trgt_fd == -1) {
		fprintf(stderr, "[ERROR] Disk.img open fail\n");
		exit(-1);
	}


	//부트로더 파일을 열고, 모든 내용을 디스크 이미지 파일로 복사
	printf("[INFO] Copy boot loader to image file\n");
	src_fd = open(argv[1], O_RDONLY);
	if (src_fd == -1) {
		fprintf(stderr, "[ERROR] %s open fail\n", argv[1]);
		exit(-1);
	}

	src_size = copy_sectors(src_fd, trgt_fd);	//error handling
	close(src_fd);
	
	//512bytes align을 위해 0x00 채우기
	bootld_sector_cnt = align_sector(trgt_fd, src_size);
	printf("[INFO] %s size = %d, Sector count = %d\n", argv[1], src_size, bootld_sector_cnt);


	//kernel32 파일을 열고 모든 내용을 디스크 이미지로 복사
	printf("[INFO] Copy Kernel32 to image file\n");
	src_fd = open(argv[2], O_RDONLY);
	if (src_fd == -1) {
		fprintf(stderr, "[ERROR] %s open fail\n", argv[2]);
		exit(-1);
	}

	src_size = copy_sectors(src_fd, trgt_fd);
	close(src_fd);
	
	k32_sector_cnt = align_sector(trgt_fd, src_size);
	printf("[INFO] %s size = %d, Sector count = %d\n", argv[2], src_size, k32_sector_cnt);

	//kernel64 파일을 열고 모든 내용을 디스크 이미지로 복사
	printf("[INFO] Copy Kernel64 to image file\n");
	src_fd = open(argv[3], O_RDONLY);
	if (src_fd == -1) {
		fprintf(stderr, "[ERROR] %s open fail\n", argv[3]);
		exit(-1);
	}

	src_size = copy_sectors(src_fd, trgt_fd);
	close(src_fd);
	
	k64_sector_cnt = align_sector(trgt_fd, src_size);
	printf("[INFO] %s size = %d, Sector count = %d\n", argv[3], src_size, k64_sector_cnt);


	//target에 kernel sector 정보 삽입
	printf("[INFO] Writing Kernel information\n");
	insert_sector_info(trgt_fd, k32_sector_cnt + k64_sector_cnt, k32_sector_cnt);
	printf("[INFO] Image file create complete\n");

	close(trgt_fd);

	return 0;
}


int align_sector(int fd, int src_size)
{
	int extra;
	int size_to_fill;
	const char dummy = 0x00;

	if (src_size < 0) {
		close(fd);
		printf("[ERROR] {fn:align_sector} invalid source size\n");
		exit(-1);
	}	

	extra = src_size % BYTE_OF_SECTOR;

	if (extra == 0)
		size_to_fill = extra;
	else
		size_to_fill = 512 - extra;

	printf("[INFO] {fn:align_sector} file size = %d, fill size = %d\n", src_size, size_to_fill);
	for (int i = 0; i < size_to_fill; i++) {
		write(fd, &dummy, 1);
	}
	
	return (src_size + size_to_fill) / BYTE_OF_SECTOR; 
}

void insert_sector_info(int bootld_fd, int total_sector_cnt, int k32_sector_cnt)
{
	unsigned short data;
	long pos; 

	pos = lseek(bootld_fd, (off_t)5, SEEK_SET);
	if (pos == -1) {
		fprintf(stderr, "[ERROR] {fn:insert_sector_info} lseek fail. Return value = %d, errno = %d, %d\n", pos, errno, SEEK_SET);
		exit(-1);
	}

	data = (unsigned short)total_sector_cnt;
	write(bootld_fd, &data, 2);

	data = (unsigned short)k32_sector_cnt;
	write(bootld_fd, &data, 2);

	printf("[INFO] {fn:insert_sector_info} Total sector count except bootloader = %d\n", total_sector_cnt);
	return;
}

int copy_sectors(int src_fd, int trgt_fd)
{
	int write_size = 0;
	int read_size = 0;
	int total_read_size = 0;
	char buffer[BYTE_OF_SECTOR] = {0};

	for (;;) {
		read_size = read(src_fd, buffer, sizeof(buffer));
		write_size = write(trgt_fd, buffer, read_size);

		if(read_size != write_size) {
			fprintf(stderr, "[ERROR] {fn:copy_sectors} read_size != write_size\n");
			//close?
			exit(-1);
		}

		total_read_size += read_size;
		if (read_size != BYTE_OF_SECTOR) {
			break;
		}
	}
	return total_read_size;
}