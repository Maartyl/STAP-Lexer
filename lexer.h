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
 
void flux_move(Flux* f, UChar c); //update flux; LF or ++

Token create_token(Flux f, Ptt type);

void print_token(FILE* fout, Token t, Flux end_pos);

void lex_all(FILE* fin, FILE* fout); //adds EOF
void lex(State state); //will lex state stream fin to state stream fout ...
#endif
