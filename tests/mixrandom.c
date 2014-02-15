#include <stdio.h>
#include <stdlib.h>
 
int main (int argc, char *argv[]) {
	FILE* ur_s = fopen("/dev/urandom","r");
	int c;
	while ((c = fgetc(stdin)) != EOF) {
		fputc(c, stdout);
		fputc(fgetc(ur_s), stdout);
	}
	fclose(ur_s); 
	return 0;
}
