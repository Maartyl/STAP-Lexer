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


FnPack fnp_find(Stt tt, UChar c); //main search point (to be used from outside)

//cascade functions: accessed through FnPack ptrs
void sf_id(State s); //does nothing
//other fns just inside...

static const FnPack fnp_id = {sf_id, sf_id}; //start point, null

#endif
