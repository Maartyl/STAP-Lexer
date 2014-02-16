#ifndef STATE_HEADER
#define STATE_HEADER
#include <stdio.h>
#include "types.h"
#include "lexer.h"
#include "cascade.h"
 
// ! other than these[st_...] fns shouldn't change state !

UChar st_firstChar(State s);
UChar st_nextChar(State s);		///reads next valid char from \fin and sets it

void st_setChar(State s, UChar c);
void st_moveFlux(State s);
void st_Fn(State s); //call
void st_Fn2(State s);
void st_setFnp(State s, FnPack fp);
void st_updateFnp(State s);
void st_setType(State s, Stt type);
void st_setToken(State s, Token tkn);
void st_setTokenType(State s, Ptt type);
void st_setTokenErr(State s);		//add ERR flag to token
void st_crtToken(State s, Ptt type);
void st_tknputc(State s);		 //tknaddc(curc);
void st_tknaddc(State s, UChar c); //token payload: add c to buffer
void st_crtBuffToken(State s, Ptt type);
void st_putcrtBuffToken(State s, Ptt type); //auto-inits with curc
void st_setPayload(State s, Buffer bf);
void st_flushToken(State s);			//prints token (and clears payload)

UChar st_getChar(State s);
Stt   st_getType(State s);

int st_matchChar(State s, char *str); //curc in str ?
int st_cmpChar(State s, char c); 	//curc == c ?

///BUG FIX - NUMR, NUMF
void st_specialized_NUMFR_stepBackFlux(State s);

/**
 * @fin stream from which to read source code characters
 * @fout stram to which print tokens
 * @returns pointer to instance of sState structure
 */
State State_new(FILE* fin, FILE* fout);
void st_finalize(State s); //free sState structure [and resources]

#endif
