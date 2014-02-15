#ifndef CASCADE_HEADER
#define CASCADE_HEADER
#include "types.h"
#include "lexer.h"
#include "state.h"
 
/*
 * 
 * dispatch  (only 1 outer fn)
 *  - fnp_find
 * 
 * all fns
 * 
 * */
 
/* NUM:
 * 
 * parse idea:
 * optional -minus
 * NUM(0..9+)									NUMI
 * on '/' -> NUM_FRAC(0..9+) ->| 				NUMR
 * on '.' -> NUM_FLOAT(0..9+)	  ->|			NUMF
 * 				on 'D' -> NUM_DBL() ->|			NUMD
 * on 'B' -> NUM_BYTE() ->|						NUMB
 * on 'L' -> NUM_LONG() ->|						NUML
 * on 'U' -> NUM_UNS(L*)				->|		NUMUI
 * 				on 'L' -> NUM_UNSLONG()s	->|	NUMUL
 * */


FnPack fnp_find(Stt tt, UChar c); //main search point (to be used from outside)

//cascade functions: accessed through FnPack ptrs
void sf_id(State s); //does nothing
//other fns just inside...

static const FnPack fnp_id = {sf_id, sf_id}; //start point, null

#endif
