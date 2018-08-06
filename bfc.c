#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bfclib/lz.h"
#include "commands.h"
#include "utils.h"
#include "constants.h"

int main(int argc, char *argv[]) {
	char *this = argv[0];
	FILE *infile;
	FILE *outfile;
	char *inname;
	char *outname;
	int optional = 0;
	int optional_val = 0;
	int ret;
	
	if(argc < 4 || argc > 5) {
		return help_cmd(this);
	}

	if(strlen(argv[1]) != 2 || argv[1][0] != '-') {
		return help_cmd(this);
	}
	inname = argv[2];
	outname = argv[3];
	
	infile = fopen(inname, "rb");
	outfile = fopen(outname, "wb");

	if(check_file(inname, infile) || check_file(outname, outfile)) {
		return -1;
	}
	if(argc == 5) {
		optional = 1;
		sscanf(argv[4], "%x", &optional_val);
	}

	/* passing the job to its handler */
	switch(argv[1][1]) {
		case COMPRESS_FC_CMD: 
			printf("Compressing '%s' to '%s' (FullComp)...\n", inname, outname);
			ret = compress_fc_cmd(infile, outfile, optional, optional_val); 
			break;
		case COMPRESS_PC_CMD: 
			printf("Compressing '%s' to '%s' (PartComp)...\n", inname, outname);
			ret = compress_pc_cmd(infile, outfile, optional, optional_val); 
			break;
		case DECOMPRESS_FC_CMD: 
			printf("Decompressing '%s'@%08x to '%s' (FullComp)...\n", inname, optional ? optional_val : 0, outname);
			ret = decompress_fc_cmd(infile, outfile, optional, optional_val); 
			break;
		case DECOMPRESS_PC_CMD: 
			printf("Decompressing '%s' to '%s' (PartComp)...\n", inname, outname);
			ret = decompress_pc_cmd(infile, outfile, optional, optional_val); 
			break;
		case TEST_CMD:
			printf("Testing in:'%s' out:'%s'...\n", inname, outname);
			ret = test_cmd(infile, outfile, optional, optional_val);
			break;
		default: ret = help_cmd(this);
	}
	printf("Done.\n");
	fclose(infile);
	fclose(outfile);
	return ret;
}
