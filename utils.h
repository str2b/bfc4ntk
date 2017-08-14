#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

int check_file(char *fname, FILE * fp);
int read_dword(FILE * fp);
void write_dword(FILE * fp, int dword);
long get_real_size(FILE *fp);
unsigned swap_endianess(unsigned int num);

#endif
