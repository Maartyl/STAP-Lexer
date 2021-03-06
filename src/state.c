#include "state.h"

UChar st_firstChar(State s){ //for flux
	UChar c = uc_read(s->fin);
	st_setChar(s, c);
	return c;
} 
  
UChar st_nextChar(State s){
	st_moveFlux(s); //use 'cur' c, not new  
	UChar c = uc_read(s->fin); //read next UTF-8 chunk of bytes
	if (c == SEPAR) return st_nextChar(s); //ignore, just move flux
	st_setChar(s, c);
	return c;
}
UChar inline st_getChar(State s){return s->curc;}
Stt  inline st_getType(State s){return s->tt;}  
 
void inline st_setChar(State s, register UChar c)  {s->curc = c;}
void inline st_moveFlux(State s)  {flux_move(&s->flux, s->curc);}
void inline st_Fn(State s)  {(*s->fp.fn)(s);}
void inline st_Fn2(State s)  {(*s->fp.fn2)(s);}
void inline st_setFnp(State s, FnPack fp)  {s->fp = fp;}
void st_updateFnp(State s)  {st_setFnp(s, fnp_find(s->tt, s->curc));}
void st_flushToken(State s)  {print_token(s->fout, s->t, s->flux); s->t.payload=NULL;}
void inline st_setType(State s, register Stt type)  {s->tt = type;}
void inline st_setPayload(State s, Buffer bf)  {s->t.payload=bf;}
void inline st_setToken(State s, Token tkn)  {s->t=tkn;}
void st_setTokenErr(State s)  {st_setTokenType(s, s->t.type|ptt_ERR);}
void inline st_setTokenType(State s, register Ptt type)  {s->t.type=type;}
void st_crtToken(State s, register Ptt type)  {st_setToken(s, create_token(s->flux, type));}
void inline st_tknputc(State s)  {st_tknaddc(s, s->curc);}
void st_tknaddc(State s, UChar c)  {
	///add each byte of UChar \c to \payload buffer in current token
	Buffer b = s->t.payload;
	void inline l(int a){strbuff_add(b, a);}
	uc_decompose(c, l); /*!*/ ///ignoring invalid \c [no better option...]
}

void st_crtBuffToken(State s, Ptt type){
	st_crtToken(s, type); 
	st_setPayload(s, strbuff_open());
}
void st_putcrtBuffToken(State s, Ptt type){
	st_crtBuffToken(s, type);
	st_tknputc(s);
}

void st_stepBackFlux(State s) {//not nice to use, if there is a better way, use that
	///BUG FIX, only works if not after enter (though at such time it would be useless)
	s->flux.pos--;
	s->flux.col--;
}

int st_matchChar(State s, char *str)  {return NULL != strchr(str, s->curc);}
int inline st_cmpChar(State s, char c)  {return c == s->curc;}



State State_new(FILE* fin, FILE* fout){
	State s = malloc(sizeof(sState));
	s->fin = fin;
	s->fout = fout;
	s->flux = (Flux){0, 1, 1}; //init: {pos, row, col}
	st_setType(s, stt_START);
	st_setChar(s, -4);    //will never be used: invalid val
	st_setFnp(s, fnp_id);
	return s;
}

void inline st_finalize(State s){
	//if I ever need to free something inside or so...
	free(s);
}
