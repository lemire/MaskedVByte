#include <stdio.h>
#include <stdlib.h>

#include "varintencode.h"
#include "varintdecode.h"

int main() {
	int N = 5000;
	uint32_t * datain = malloc(N * sizeof(uint32_t));
	uint8_t * compressedbuffer = malloc(N * sizeof(uint32_t));
	uint32_t * recovdata = malloc(N * sizeof(uint32_t));

	for (int length = 0; length <= N; ++length) {
		for (int gap = 1; gap <= 387420489; gap *= 3) {
			for (int k = 0; k < length; ++k)
				datain[k] = gap;
			size_t compsize = vbyte_encode(datain, length, compressedbuffer);
			size_t usedbytes = masked_vbyte_decode(compressedbuffer, recovdata,
					length);
			if (compsize != usedbytes) {
				printf("code is buggy");
				return -1;
			}
			for (int k = 0; k < length; ++k) {
				if (recovdata[k] != datain[k]) {
					printf("code is buggy");
					return -1;
				}
			}
			size_t decodedints = masked_vbyte_decode_fromcompressedsize(compressedbuffer, recovdata,
					compsize);
			if (decodedints != (size_t) length) {
				printf("code is buggy");
				return -1;
			}
			for (int k = 0; k < length; ++k) {
				if (recovdata[k] != datain[k]) {
					printf("code is buggy");
					return -1;
				}
			}
		}
	}
	free(datain);
	free(compressedbuffer);
	free(recovdata);
	printf("Code looks good.\n");
	return 0;
}

