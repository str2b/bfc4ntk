#include "utils.h"

int check_file(char *fname, FILE * fp) {
	if(fp == 0) {
		perror(fname);
		return 1;
	}
	return 0;
}

int read_dword(FILE * f) {
	unsigned char buf[4];
	fread(buf, 4, 1, f);
	return (((unsigned int) buf[0]) << 24) +
		(((unsigned int) buf[1]) << 16) +
		(((unsigned int) buf[2]) << 8) +
		(unsigned int) buf[3];
}

void write_dword(FILE * f, int x) {
	fputc((x >> 24) & 255, f);
	fputc((x >> 16) & 255, f);
	fputc((x >> 8) & 255, f);
	fputc(x & 255, f);
}

long get_real_size(FILE *fp) {
	long size, pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return size;
}

unsigned swap_endianess(unsigned int num) {
	return ((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000);	
}
