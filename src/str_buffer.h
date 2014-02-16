#ifndef STR_BUFFER_HEADER
#define STR_BUFFER_HEADER
#include <stdio.h>
#include <stdlib.h>
 
typedef struct sBuff {
	char * str;
	size_t size;
	FILE * buff;
} sBuffer;
typedef sBuffer* Buffer;

Buffer strbuff_open();
void strbuff_add(Buffer b, int c);
char* strbuff_close(Buffer b, /*out*/ size_t *size);
//needs to free resulting char[]

#endif
