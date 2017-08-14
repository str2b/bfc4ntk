#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "constants.h"
#include "utils.h"
#include "bfclib/lz.h"

int help_cmd(char *arg0) {
	printf("%s v%s (%s) by %s\n(Ntk firmware packer/unpacker)\n\n", NAME, VERSION, BUILD_DATE, AUTHOR);
	printf("Released for:\n%s\n", RELEASED_FOR);
	printf("\n");
	printf("Usage:\n");
	printf("  %s <mode> <infile> <outfile> [optional]", arg0);
	printf("\n\n");
	printf("Modes:\n");
	printf("  -%c: Compress file (FullComp):\n", COMPRESS_FC_CMD);	
	printf("       * -%c <infile> <outfile>\n\n", COMPRESS_FC_CMD);
	printf("  -%c: Compress file (PartComp):\n", COMPRESS_PC_CMD);
	printf("       * -%c <infile> <outfile> [PartComp offset, default: <auto detect>]\n\n", COMPRESS_PC_CMD);
	printf("  -%c: Decompress file (FullComp, 1 partition):\n", DECOMPRESS_FC_CMD);
	printf("       * -%c <infile> <outfile> [FullComp header offset, default: 0x%08x]\n\n", DECOMPRESS_FC_CMD, 0);
	printf("  -%c: Decompress file (PartComp, 1 partition):\n", DECOMPRESS_PC_CMD);
	printf("       * -%c <infile> <outfile> [PartComp header offset, default: <auto detect>]\n\n", DECOMPRESS_PC_CMD);
	return EXIT_FAILURE;
}

int prepare_char_buf(unsigned char **buf, int bufsize) {
	*buf = malloc(bufsize);
	if(!*buf) {
		perror("Cannot allocate enough memory");
		return 0;
	}
	return 1;
}

void write_bcl_header(FILE *outfile, long offset, char magic[4], int algo, int insize, int outsize) {
	fseek(outfile, offset, SEEK_SET);
	fwrite(magic, 4, 1, outfile);
	write_dword(outfile, algo);
	write_dword(outfile, insize); //raw size
	write_dword(outfile, outsize); //compressed size
}

void patch_ntk_header(FILE *outfile, int full_size, int binctrl_flag) {
	fseek(outfile, FILESIZE_OFFSET, SEEK_SET);
	write_dword(outfile, swap_endianess(full_size));
	
	fseek(outfile, BINCTRL_OFFSET, SEEK_SET);
	write_dword(outfile, swap_endianess(binctrl_flag)); //patch to raw/FullComp binary
}

/* returns new outsize */
int fix_ntk_alignment(FILE * outfile, int outsize) {
	int tmp;
	if((tmp = outsize % DWORD_SIZE)) {
		int fill_bytes = DWORD_SIZE - tmp;
		outsize += fill_bytes;
		
		int i;
		for(i = 0; i < fill_bytes; i++) {
			fputc('\0', outfile);
		}
	}
	return outsize;
}

unsigned char *compress_file_at(FILE *infile, int insize, long offset, int *outsize) {
	unsigned char *inbuf, *outbuf;
	fseek(infile, offset, SEEK_SET);
	/* allocate output buffer with worst case size */
    *outsize = (insize * 104 + 50) / 100 + 384;
	if(!prepare_char_buf(&outbuf, *outsize))
		return 0;

	/* read file to buffer*/
	if(!prepare_char_buf(&inbuf, insize)) 
		return 0;
	fread(inbuf, insize, 1, infile);

	printf("Creating compressed portion at %08x\n", (int) offset);
	/* compress using bfc lz77 */
	unsigned int *work = malloc(sizeof(unsigned int) * (65536 + insize));
	if (work) {
		*outsize = LZ_CompressFast(inbuf, outbuf, insize, work);
		free(work);
	} else {
		*outsize = LZ_Compress(inbuf , outbuf, insize);
	}
	if(*outsize < 1) {
		printf("Error: Nothing to compress!");
		free(inbuf);
		free(outbuf);
		return 0;
	}
	free(inbuf);
	return outbuf;
}

int compress_fc_cmd(FILE *infile, FILE *outfile, int optional, int optional_val) {
	unsigned char *outbuf;
	int insize = get_real_size(infile), outsize = 0;
	int offset = 0;
	
	outbuf = compress_file_at(infile, insize, offset, &outsize);
	if(!outbuf) {
		printf("Error: Compression failed!");
		return -1;
	}
	
	/* write compressed file */
	fseek(outfile, HEADER_LEN, SEEK_SET);
	fwrite(outbuf, outsize, 1, outfile);
	
	/* fix Ntk alignment */
	outsize = fix_ntk_alignment(outfile, outsize);
		
	/* write bcl header */
	write_bcl_header(outfile, 0, BCL_MAGIC, LZ_ALGO, insize, outsize);
	
	free(outbuf);
	
	return 0;
}

int compress_pc_cmd(FILE *infile, FILE *outfile, int optional, int optional_val) {
	unsigned char *outbuf, *cpy;
	int insize = get_real_size(infile), outsize = 0, fixed_outsize;
	int computed_offset, offset;
	
	fseek(infile, PC_OFFS_OFFSET, SEEK_SET);
	computed_offset = swap_endianess(read_dword(infile));
	if(!optional && (computed_offset < 0 || computed_offset > insize)) {
		printf("Error: File size is abnormal\n");
		return -1;
	}
	offset = (optional ? optional_val : computed_offset);
	
	
	outbuf = compress_file_at(infile, insize - offset, offset, &outsize);
	if(!outbuf) {
		printf("Error: Compression failed!");
		return -1;
	}
	
	if(!prepare_char_buf(&cpy, offset)) {
		return -1;
	}
	
	fseek(infile, 0, SEEK_SET);
	fread(cpy, sizeof(char), offset, infile);
	fwrite(cpy, offset, 1, outfile);
	write_bcl_header(outfile, offset, BCL_MAGIC, LZ_ALGO, insize - offset, outsize);
	fwrite(outbuf, outsize, 1, outfile);
	
	/* Fix header */
	fixed_outsize = fix_ntk_alignment(outfile, offset + HEADER_LEN + outsize);
	patch_ntk_header(outfile, fixed_outsize, 1);
	fseek(outfile, FILESIZE_OFFSET, SEEK_SET);
	write_dword(outfile, swap_endianess(fixed_outsize));
	
	free(outbuf);
	return 0;
}

unsigned char *uncompress_file_at(FILE *infile, int insize, long offset, int *outsize) {
	int insize_stored;
	unsigned char *inbuf, *outbuf;
	int start_offset = offset;
	char magic[5];
	magic[4] = 0;
	

	/* safety checks */
	fseek(infile, start_offset + MAGIC_OFFSET, SEEK_SET);
	fread(magic, 1, 4, infile);
	if(strcmp(BCL_MAGIC, magic)) {
		printf("Error: Invalid magic constant: '%s'\n", magic);
		return 0;
	}
	fseek(infile, start_offset + ALGO_OFFSET, SEEK_SET);
	int algo = read_dword(infile) & 0x0000FFFF;
	if(algo != LZ_ALGO) {
		printf("Error: Unsupported algorithm: %04x\n", algo);
		return 0;
	}
	*outsize = read_dword(infile);
	insize_stored = read_dword(infile);
	if((insize < insize_stored + HEADER_LEN)) {
		printf("Error: File size is abnormal (is:%08x issh:%08x)\n", insize, insize_stored + HEADER_LEN);
		return 0;
	}
	
	/* read file to buffer*/
	if(!prepare_char_buf(&inbuf, insize))
		return 0;
	fread(inbuf, insize, 1, infile);
	
	/* Allocate memory for output buffer */
	if(!prepare_char_buf(&outbuf, *outsize))
		return 0;
	
	/* uncompress via bfc lib */
	LZ_Uncompress(inbuf, outbuf, insize);
	
	free(inbuf);
	return outbuf;
}

int decompress_fc_cmd(FILE *infile, FILE *outfile, int optional, int optional_val) {
	int outsize, insize = get_real_size(infile);
	unsigned char *outbuf = uncompress_file_at(infile, insize, (optional ? optional_val : 0), &outsize);
	if(!outbuf) {
		return -1;
	}
	/* write outfile */
	fwrite(outbuf, outsize, 1, outfile);
	free(outbuf);
	return 0;
}

int decompress_pc_cmd(FILE *infile, FILE *outfile, int optional, int optional_val) {
	int outsize, insize = get_real_size(infile), pc_offset, offset;
	unsigned char *cpy;
	char pc_check[5];
	pc_check[4] = 0;
	fread(pc_check, sizeof(char), 4, infile);
	if(!strcmp(pc_check, BCL_MAGIC)) {
		printf("Error: File is not a PartComp binary!\n");
		return -1;
	}
	fseek(infile, PC_OFFS_OFFSET, SEEK_SET);
	pc_offset = swap_endianess(read_dword(infile));
	offset = (optional ? optional_val : pc_offset);

	if(offset > insize) {
		printf("Error: File size is abnormal\n");
		return -1;
	}
	
	if(!prepare_char_buf(&cpy, offset)) {
		return -1;
	}
	fseek(infile, 0, SEEK_SET);
	fread(cpy, sizeof(char), offset, infile);
	printf("Using offset %08x\n", offset);
	unsigned char *outbuf = uncompress_file_at(infile, insize - offset, offset, &outsize);
	if(!outbuf) {
		free(cpy);
		return -1;
	}
	fwrite(cpy, offset, 1, outfile);
	fwrite(outbuf, outsize, 1, outfile);
	
	/* Fix raw header*/
	patch_ntk_header(outfile, outsize + offset, 0);
	
	free(outbuf);
	free(cpy);
	return 0;
}

int test_cmd(FILE *infile, FILE *outfile, int optional, int optional_val) {
	printf("No test command implemented at the moment!\n");
	return -1;
}
