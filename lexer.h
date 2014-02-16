#ifndef LEXER_HEADER
#define LEXER_HEADER
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "str_buffer.h"
#include "uchar.h"
#include "state.h"
#include "cascade.h"
 
///important parts to be changed maybe [format, line numbers...]
///+Token
///+Lex fns. itself
 
void flux_move(Flux* f, UChar c); //update flux; LF ignore, otherwise ++

Token create_token(Flux f, Ptt type);

///closes buffer, letting @f use string
void close_token(Token t, void (*f)(char*, size_t));

void print_token(FILE* fout, Token t, Flux end_pos);

///closes buffer without printing - frees
void cancel_token(Token t);

void lex_all(FILE* fin, FILE* fout); //adds EOF

///main loop
void lex(State state); //will lex state stream fin to state stream fout ...
#endif
