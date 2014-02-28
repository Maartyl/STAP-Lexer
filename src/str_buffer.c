#include "str_buffer.h"

Buffer strbuff_open(){
	Buffer b = malloc(sizeof(sBuffer));
	b->str = NULL; b->size = 0;
	b->buff = open_memstream(&b->str,&b->size);
	return b;
}
void strbuff_add(Buffer b, int c){
	fputc(c, b->buff);
} 
char *strbuff_close(Buffer b, /*out*/ size_t *size){
	strbuff_add(b, 0); //null terminated memory segment
	
	fclose(b->buff); //frees itself 
	*size = b->size;
	char *cr = b->str;
	free(b);
	return cr;
}
