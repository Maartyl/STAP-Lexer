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
UChar st_getChar(State s){return s->curc;}
Stt  st_getType(State s){return s->tt;}  
 
void st_setChar(State s, UChar c)  {s->curc = c;}
void st_moveFlux(State s)  {flux_move(&s->flux, s->curc);}
void st_Fn(State s)  {(*s->fp.fn)(s);}
void st_Fn2(State s)  {(*s->fp.fn2)(s);}
void st_setFnp(State s, FnPack fp)  {s->fp = fp;}
void st_updateFnp(State s)  {st_setFnp(s, fnp_find(s->tt, s->curc));}
void st_flushToken(State s)  {print_token(s->fout, s->t, s->flux); s->t.payload=NULL;}
void st_setType(State s, Stt type)  {s->tt = type;}
void st_setPayload(State s, Buffer bf)  {s->t.payload=bf;}
void st_setToken(State s, Token tkn)  {s->t=tkn;}
void st_setTokenErr(State s)  {st_setTokenType(s, s->t.type|ptt_ERR);}
void st_setTokenType(State s, Ptt type)  {s->t.type=type;}
void st_crtToken(State s, Ptt type)  {st_setToken(s, create_token(s->flux, type));}
void st_tknputc(State s)  {st_tknaddc(s, s->curc);}
void st_tknaddc(State s, UChar c)  {
	///add each byte of UChar \c to \payload buffer in current token
	Buffer b = s->t.payload;
	void l(int a){strbuff_add(b, a);}
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


int st_matchChar(State s, char *str)  {return NULL != strchr(str, s->curc);}
int st_cmpChar(State s, char c)  {return c == s->curc;}



State State_new(FILE* fin, FILE* fout){
	State s = malloc(sizeof(sState));
	s->fin = fin;
	s->fout = fout;
	s->flux = (Flux){0, 1, 1}; //init: {pos, row, col}
	st_setType(s, stt_NONE);
	st_setChar(s, -4);    //will never be used: invalid val
	st_setFnp(s, fnp_id);
	return s;
}

void st_finalize(State s){
	//if I ever need to free something inside or so...
	free(s);
}
