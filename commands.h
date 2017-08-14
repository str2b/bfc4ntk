#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdio.h>

#define COMPRESS_FC_CMD 'c'
#define COMPRESS_PC_CMD 'p'
#define DECOMPRESS_FC_CMD 'd'
#define DECOMPRESS_PC_CMD 'x'
#define TEST_CMD 't'

int compress_fc_cmd(FILE *infile, FILE *outfile, int optional, int optional_val);
int compress_pc_cmd(FILE *infile, FILE *outfile, int optional, int optional_val);
int decompress_fc_cmd(FILE *infile, FILE *outfile, int optional, int optional_val);
int decompress_pc_cmd(FILE *infile, FILE *outfile, int optional, int optional_val);
int test_cmd(FILE *infile, FILE *outfile, int optional, int optional_val);
int help_cmd(char *arg0);

#endif
