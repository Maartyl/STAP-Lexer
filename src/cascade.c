#include "cascade.h"

//<helpers> //update for Unicode ?
static int is_any_of(UChar c, char* pattern){return (strchr(pattern, c) != NULL);}
static int is_num(UChar c){return c>='0' && c<='9';}
static int inline is_num_first(UChar c){return is_num(c);}//_-_ is elsewhere [1st class problem]
static int is_id(UChar c){return 
	(c>='0' && c<='9') || (c>='a' && c<='z') || 
	(c>='A' && c<='Z') || is_any_of(c, "_$");}
static int is_id_first(UChar c){return 
	(c>='a' && c<='z') || (c>='A' && c<='Z') || is_any_of(c, "_$");}
static int inline is_symbol(UChar c){return is_any_of(c, "+-*/&^<>~|=?!.%#");}
static int inline is_esc(UChar c){return is_any_of(c, "abfnrtv0\\\"\'");}
static int inline is_openable(UChar c){return !is_any_of(c, " \n\t,;");} //whitespace, CMNT atp.
//</helpers>

//<all fns [StepFn+helperes+combiners]>

void inline  sf_id(State s){/*pass*/} //not static
static void sf_reset(State s){ //no longer continues
	DEBUG(char* str = uc_toStr(st_getChar(s));
	printf("sf_reset(%d, %s)\n", st_getType(s), str[0] == '\n' ? "ENTER" : str);
	if (strlen(str)) free(str);)//DEBUG
	
	st_setType(s, stt_NONE);
}
static void inline sf_flush(State s){st_flushToken(s);}
static void sf_flush_reset(State s){
	sf_flush(s);
	sf_reset(s); //flush also resets: token is done and has no consequence.
}

///fall through
static void sf_flush_recur(State s)  {
	///MUST NOT BE CALLED FROM stt_NONE branch! [infinite loop]
	DEBUG(puts("RECUR");)
	sf_flush_reset(s); //is done, flushes; searches with NONE
	st_updateFnp(s); //recursive (not really, but in a sense)
	st_Fn(s); 	//call now, for it would skip a char otherwise
				//for this Fn is called instead
}

static FnPack inline with_flush(StepFn sf){return (FnPack){sf, sf_flush};}
static FnPack inline with_flush_reset(StepFn sf){return (FnPack){sf, sf_flush_reset};}
static FnPack inline with_id(StepFn sf){return (FnPack){sf, sf_id};}

FnPack fnp_reset = {sf_reset, sf_id};
FnPack fnp_flush_recur = {sf_flush_recur, sf_id};
//FnPack fnp_id = {sf_id, sf_id}; //already exists [in header]

/// -- ACTUAL Fns ARE FROM NOW ON

 
#define F_SET_TOKEN(TKN) static void inline sf_set_##TKN(State s){st_crtToken(s, ptt_##TKN);}
///[](){}
F_SET_TOKEN(OP) F_SET_TOKEN(OB) F_SET_TOKEN(OC) //with_flush_reset
F_SET_TOKEN(CP) F_SET_TOKEN(CB) F_SET_TOKEN(CC)

///STR (start, end)
static void sf_str_start(State s){ //on " encounter //w flush
	st_setType(s, stt_STR); 
	st_crtToken(s, ptt_STRO);
}
static void sf_str_end(State s){ //on _"_ encounter in stt_STR (not stt_STRESC) //w flush
	st_setType(s, stt_NONE); 
	st_crtToken(s, ptt_STRC);
}
//--str helpers
static void sf_str_reset(State s){ //no longer continues
	DEBUG(char* str = uc_toStr(st_getChar(s));
	printf("sf_str_reset(%d, %s)\n", st_getType(s), str);
	if (strlen(str)) free(str);)//DEBUG
	
	st_setType(s, stt_STR);
}
static void sf_str_flush_reset(State s){
	sf_flush(s);
	sf_str_reset(s);
}
static void sf_str_flush_recur(State s){ //no flush
	///MUST NOT BE CALLED FROM stt_STR branch! [infinite loop]
	///just like sf_flush_recur[recur], but returns to STR
	DEBUG(puts("RECUR STR");)
	sf_str_flush_reset(s); //is done, flushes; searches with STR
	st_updateFnp(s); //recursive (not really, but in a sense)
	st_Fn(s); 	//call now, for it would skip a char otherwise
				//for this Fn is called instead
}
static void sf_str_err_recur(State s){ //no flush
	///just add error and recur normaly
	DEBUG(puts("ESC ERR");)
	st_setTokenErr(s);
	sf_str_flush_recur(s);
}
FnPack fnp_str_err_recur = {sf_str_err_recur, sf_id};
FnPack fnp_str_flush_recur = {sf_str_flush_recur, sf_id};
///STRD __ 
static void sf_strd_start(State s){//no flush 
	st_setType(s, stt_STRD);
	st_putcrtBuffToken(s, ptt_STRD);
}
static void inline sf_strd_step(State s){st_tknputc(s);}//no flush

///STRESC __ 
static void sf_stresc_start(State s){//no flush
	st_setType(s, stt_STRESC);
	st_crtBuffToken(s, ptt_STRESC); //empty, just data later
}
static void sf_stresc_one(State s){//w flush, no reset
	st_setType(s, stt_STR);
	st_tknputc(s);
}
static void inline sf_just_minus(State s){st_setType(s, stt_MINUS);}//no flush

///NUM

static void sf_num_start(State s){//no flush
	st_setType(s, stt_NUM);
	st_putcrtBuffToken(s, ptt_NUMI);
}
static void sf_minus_num_start(State s){//no flush
	st_setType(s, stt_NUM);
	st_crtBuffToken(s, ptt_NUMI);
	st_tknaddc(s, '-');
	st_tknputc(s);
}
static void sf_num_step(State s){st_tknputc(s);}//no flush
//--
static void sf_numD(State s){st_setTokenType(s, ptt_NUMD);}  //w flush reset
static void sf_numB(State s){st_setTokenType(s, ptt_NUMB);}  //w flush reset
static void sf_numL(State s){st_setTokenType(s, ptt_NUML);}  //w flush reset
static void sf_numUL(State s){st_setTokenType(s, ptt_NUMUL);}//w flush reset
//--
static void sf_numR(State s){//w 2
	DEBUG(puts("sf_numR");)
}
static void sf_numF(State s){//w 2
	DEBUG(puts("sf_numF");)
}
//--
static void sf2_numR(State s){ //first time
	DEBUG(printf("sf2_numR");)
	if (is_num(st_getChar(s))) {
		st_tknaddc(s, '/');
		st_setType(s, stt_NUMR);
		st_setTokenType(s, ptt_NUMR);
	}	
	else { //rollback, isn't Rational, but just Int with / after itself
		DEBUG(puts(" ROLLBACK start");)
		sf_flush_reset(s);
		UChar c = st_getChar(s); //postpone curc, for last wasn't performed - simulate
		st_setChar(s, '/');
		st_updateFnp(s);
		st_Fn(s);
		st_setChar(s, c); //reapply curc and perform this Fn2, but updated, 
		st_Fn2(s);        //returning to normal cycle ...
		DEBUG(puts("ROLLBACK end");)
	}
}
static void sf2_numF(State s){ //first time
	DEBUG(printf("sf2_numF");)
	if (is_num(st_getChar(s))) {
		st_tknaddc(s, '.');
		st_setType(s, stt_NUMF);
		st_setTokenType(s, ptt_NUMF);
	}	
	else { //rollback, isn't Float, but just Int with . after itself
		DEBUG(puts(" ROLLBACK start");)
		sf_flush_reset(s);
		UChar c = st_getChar(s); //postpone curc, for last wasn't performed - simulate
		st_setChar(s, '.');
		st_updateFnp(s);
		st_Fn(s);
		st_setChar(s, c); //reapply curc and perform this Fn2, but updated, 
		st_Fn2(s);        //returning to normal cycle ...
		DEBUG(puts("ROLLBACK end");)
	}
}
//--
FnPack fnp_numR = {sf_numR, sf2_numR};
FnPack fnp_numF = {sf_numF, sf2_numF};
//--
static void sf_numU(State s){//w id
	st_setType(s, stt_NUMU);
	st_setTokenType(s, ptt_NUMUI);
}

///SYMBOL
static void sf_symbol_start(State s){//step_flush: if len=1
	st_setType(s, stt_SYMBOL);
	st_putcrtBuffToken(s, ptt_SYMBOL);
}
static void sf_minus_symbol_start(State s){//no flush
	DEBUG(puts("sf_minus_symbol_start");)
	st_setType(s, stt_SYMBOL);
	st_crtBuffToken(s, ptt_SYMBOL);
	st_tknaddc(s, '-');
	st_tknputc(s);
}
static void sf_minus_symbol_only(State s){//no flush
	DEBUG(puts("sf_minus_symbol_only");)
	st_crtBuffToken(s, ptt_SYMBOL);
	st_tknaddc(s, '-');
	sf_flush_recur(s); //recur: actually apply curc, is not part of symbol
}
static void inline sf_symbol_step(State s)  {st_tknputc(s);}//no flush

///ID  identifiers
static void sf_id_start(State s){//no flush
	st_setType(s, stt_ID);
	st_putcrtBuffToken(s, ptt_ID);
}
static void inline sf_id_step(State s)  {st_tknputc(s);}//no flush

///OPEN [opener characteres like : # ...]
static void sf_open(State s){//w id, then flush recur
	st_setType(s, stt_OPEN);
	st_putcrtBuffToken(s, ptt_OPEN);
} 
//could become symbol... [whitespace etc. after it]
static void sf_open_symbol(State s){//instead of flush recur of [sf_open]
	st_setTokenType(s, ptt_SYMBOL);
	sf_flush_recur(s); //call flush recur
} 

///FUNCTION "\", "\\"
static void sf_fnl(State s) {
	st_setType(s, stt_FN);
	st_crtToken(s, ptt_FNL);
}
static void inline sf_fnn(State s) {st_setTokenType(s, ptt_FNN);}

///COMMENT [ ; ... \n ]
static void sf_cmnt_start(State s){//w id, uses universal end
	st_setType(s, stt_CMNT);
	st_crtToken(s, ptt_CMNT);
}

///SHEBANG - starts if first char is #, if second is ! turns into CMNT
static void sf_shebang(State s){//w id
	st_setType(s, stt_SHEBANG);
	st_crtToken(s, ptt_OPEN);
}
static void sf2_shebang(State s){//w id
	if (!st_cmpChar(s, '!')) {
		st_setPayload(s, strbuff_open());
		st_tknaddc(s, '#');
	}
}
FnPack fnp_shebang = {sf_shebang, sf2_shebang};
static void sf_shebang_cmnt(State s){//w id
	st_setType(s, stt_CMNT);
	st_setTokenType(s, ptt_CMNT);
}





//</all fns>

static FnPack triforce_find(Stt tt, UChar c){ //+num, symbol, id
	if(is_num_first(c)) return with_id(sf_num_start); //NUM
	if(is_symbol(c)) return with_id(sf_symbol_start);   //SYMBOL
	if(is_id_first(c)) return with_id(sf_id_start); //ID
	DEBUG(puts("triforce through");) ///minimize this [TODO solve unicode IDs here]
	return fnp_id;
}

static FnPack minus_find(Stt tt, UChar c){ //-num, symbol
	if(is_num_first(c)) return with_id(sf_minus_num_start); //NUM
	if(is_symbol(c)) return with_id(sf_minus_symbol_start); //SYMBOL (starts with -)
	return with_id(sf_minus_symbol_only); //SYBMOL: just -
}
 
FnPack fnp_find(Stt tt, UChar c){			///main enetery

	DEBUG(char* str = uc_toStr(c);
	printf("fnp_find(%d, %s)\n", tt, str[0] == '\n' ? "ENTER" : str); 
	if (strlen(str)) free(str);)//DEBUG
	
	//breaks after returns not needed. [omitted on purpouse]
	switch(tt) {
		case stt_STR: switch(c) {
			case -1 : return fnp_id; //EOF: just end, don't care abou tgrowing err
			case '"': return with_flush(sf_str_end); //wo reset
			case '\\':return with_id(sf_stresc_start);
			default : return with_id(sf_strd_start);
		} break;
		case stt_STRD: switch(c) {
			case -1 : //EOF: invalid string, but file STRD part...
			case '"': 
			case '\\':return fnp_str_flush_recur; //STRD ended, not whole string
			default : return with_id(sf_strd_step);
		} break;
		case stt_STRESC: return c=='x'
				? fnp_str_err_recur //for now... [files empty ESC]
				: (is_esc(c)
					? with_flush(sf_stresc_one) //no reset: sets STR
					: fnp_str_err_recur); //file error and retry as STR

		case stt_FN: return c=='\\'
				? with_flush_reset(sf_fnn)
				: fnp_flush_recur; break;
				
		case stt_OPEN: return is_openable(c)
				? fnp_flush_recur
				: with_id(sf_open_symbol);
		case stt_MINUS: return minus_find(tt, c);
		case stt_ID: return is_id(c) 
				? with_id(sf_id_step)
				: fnp_flush_recur;  break;
		case stt_SYMBOL: return is_symbol(c) 
				? with_id(sf_symbol_step)
				: fnp_flush_recur; break;
		case stt_NUM: switch(c) {
			case '.': return fnp_numF;
			case '/': return fnp_numR;
			case 'B': return with_flush_reset(sf_numB);
			case 'L': return with_flush_reset(sf_numL);
			case 'U': return with_id(sf_numU);
			default:  return is_num(c) 
				? with_id(sf_num_step) //just append
				: fnp_flush_recur ;//fall through, flush and repeat
		} break;
		case stt_NUMR: return is_num(c)
				? with_id(sf_num_step)
				: fnp_flush_recur; //done, no longer number
		case stt_NUMF: return is_num(c)
				? with_id(sf_num_step)		//just append c
				: (c=='D'
					? with_flush_reset(sf_numD)	//is double
					: fnp_flush_recur);
		case stt_NUMU: return c=='L'
				? with_flush_reset(sf_numUL)
				: fnp_flush_recur;		//just unsigned integer
				
		case stt_CMNT: return (c=='\n')||(c==-1) //end wth enter or EOF
				? with_flush_reset(sf_id)
				: fnp_id; //ignore
		
		case stt_NONE: switch(c) { 
			case '(': return with_flush_reset(sf_set_OP); 
			case ')': return with_flush_reset(sf_set_CP); 
			case '[': return with_flush_reset(sf_set_OB); 
			case ']': return with_flush_reset(sf_set_CB); 
			case '{': return with_flush_reset(sf_set_OC); 
			case '}': return with_flush_reset(sf_set_CC); 
			case '"': return with_flush(sf_str_start); 
			case '-': return with_id(sf_just_minus); 
			case '\\':return with_id(sf_fnl);
			case ';': return with_id(sf_cmnt_start);
			case ':': 
			case '\'':
			case '@':
			case '_':
			case '`': ///ALL OPENS HERE
			case '#': return with_id(sf_open);
			case ' ':
			case ',':
			case '\t':
			case '\n':return fnp_id; //just ignore, might be used in future...
			default : return triforce_find(tt, c); //no need to check for Opens
		} break;
		
		case stt_START: return c=='#'
				? fnp_shebang
				: fnp_find(stt_NONE, c);
		case stt_SHEBANG: return c=='!'
				? with_id(sf_shebang_cmnt)
				: fnp_flush_recur;  //flush #[Open] and retry for !
		
		
		default: return fnp_flush_recur; //error, try to fix... //shouldn't ever happen
	}
}

