#include <stdio.h>
#include <stdlib.h>

#include "varintencode.h"
#include "varintdecode.h"

int main() {
	int N = 5000;
	uint32_t * datain = malloc(N * sizeof(uint32_t));
	uint8_t * compressedbuffer = malloc(N * sizeof(uint32_t));
	uint32_t * recovdata = malloc(N * sizeof(uint32_t));
	for (int k = 0; k < N; ++k)
		datain[k] = 140;
	size_t compsize = vbyte_encode(datain, N, compressedbuffer);
	masked_vbyte_decode(compressedbuffer, recovdata,
					N);
	free(datain);
	free(compressedbuffer);
	free(recovdata);
	printf("Compressed %d integers down to %d bytes.\n",N,(int) compsize);
	return 0;
}

