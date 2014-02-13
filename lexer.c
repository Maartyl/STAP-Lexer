#include "lexer.h"

void flux_move(Flux* f, UChar c){
	if (c == 13) /*CR*/ return; //change nothing, ignore
	f->pos++;
	if (c != 10) f->col++; else { //newline - LF
		f->row++;
		f->col = 1;
	} 
}

Token create_token(Flux f, Ptt type){
	Token st;
	st.flux = f; //copy, not reference
	st.type = type;
	st.payload = NULL; //can be set later, if needed
	return st;
}

void print_token(FILE* fout, Token t, Flux end_pos){
	Flux fl = t.flux;
	size_t payload_len = 0; 
	///str_buff rslt is null-terminated
	//puts("print_token_0");
	char* str = t.payload == NULL ? "" : strbuff_close(t.payload, &payload_len);
	//puts("print_token_1");
	fprintf(fout, "\t\t%d,%d,%d,%d,%d|%s%c\n", //\t\t, \n just for debug
		fl.pos, end_pos.pos-fl.pos, fl.row, fl.col, t.type, str, SEPAR);
	if (payload_len != 0) free(str);
}

void lex_all(FILE* fin, FILE* fout) {
	State s = State_new(fin, fout);
	lex(s);
	st_crtToken(s, ptt_EOF);
	st_flushToken(s);
	
	st_finalize(s);
}

void lex(State s){
	if (st_firstChar(s) == EOF) return; //maybe stupid: flux problem fix
	if (!st_cmpChar(s, SEPAR)){
		printf("loop \\ %c\n", st_getChar(s)); //debug
		st_updateFnp(s);
		st_Fn(s);
	}
	//TODO solve shebang #! ... //as Stt
	while (st_nextChar(s) != EOF) {	//__ main loop __
		st_Fn2(s); 
		/*old debug*/ //char* str = uc_toStr(st_getChar(s)); printf("loop \\ %s\n", str[0] == '\n' ? "ENTER" : str);if (strlen(str)) free(str);//debug
		st_updateFnp(s);
		st_Fn(s);
	}
	st_Fn2(s); //didn't enter anymore
	
	//to ensure that all will finalize
	if (st_getType(s) == stt_NONE) return; //has no state, nothing to finish...
	st_setChar(s, -1); //EOF, can't match anything new
	st_updateFnp(s); 
	st_Fn(s);
	st_Fn2(s);
}










