#include <stdio.h>
#include <unistd.h> //getopt
#include "lexer.h"

 
int main (int argc, char *argv[]) {
	FILE* fin = stdin;
	FILE* fout = stdout;
	
	//<args>
	int c;
	opterr = 0;
	while ((c = getopt (argc, argv, "i:o:")) != -1)
		switch (c) {
			case 'i': fin = fopen(optarg, "r"); break;
			case 'o': fout = fopen(optarg, "w"); break;
			default: abort ();
		}
	if (opterr != 0) return opterr; //exit
	//</args>
	
	
	lex_all(fin, fout);

	fclose(fout); //flush stream
	fclose(fin); //pointless, but if it is a file...
	return 0;
}
