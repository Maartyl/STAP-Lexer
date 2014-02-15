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

void close_token(Token t, void (*f)(char*, size_t)){
	size_t payload_len = 0; 
	char* str = t.payload == NULL ? "" : strbuff_close(t.payload, &payload_len);
	(*f)(str, payload_len);
	if (payload_len != 0) free(str); //0 for static ""
}

void print_token(FILE* fout, Token t, Flux end_pos){
	void l(char* str, size_t len){
		Flux fl = t.flux;
		fprintf(fout, "\t\t%d,%d,%d,%d,%d|%s%c\n", //\t\t, \n just for debug
			fl.pos, end_pos.pos-fl.pos, fl.row, fl.col, t.type, str, SEPAR);
	}
	close_token(t, l);
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
		st_updateFnp(s);
		st_Fn(s);
	}
	st_Fn2(s); //didn't enter anymore
	
	//to ensure that all will finalize
	if (st_getType(s) == stt_NONE) return; //NONE has no state, nothing to finish...
	st_setChar(s, -1); //EOF, can't match anything new
	st_updateFnp(s); 
	st_Fn(s);
	st_Fn2(s);
}










