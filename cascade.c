#include "cascade.h"

//<helpers> //update for Unicode ?
int is_num(UChar c){return c>='0' && c<='9';}
int is_num_first(UChar c){return is_num(c);}//_-_ is elsewhere [1st class problem]
int is_id(UChar c){return 
	(c>='0' && c<='9') || (c>='a' && c<='z') || 
	(c>='A' && c<='Z') || (strchr("_$", c) != NULL);}
int is_id_first(UChar c){return 
	(c>='a' && c<='z') || (c>='A' && c<='Z') || (strchr("_$", c) != NULL);}
int is_symbol(UChar c){return strchr("+-*/&^<>~|=?!.%#", c) != NULL;}
int is_esc(UChar c){return strchr("abfnrtv0\\\"\'", c) != NULL;}
//</helpers>

//<all fns [StepFn+helperes+combiners]>

void sf_id(State s){/*pass*/}
void sf_reset(State s){ //no longer continues
	char* str = uc_toStr(st_getChar(s));
	printf("sf_reset(%d, %s)\n", st_getType(s), str[0] == '\n' ? "ENTER" : str);
	if (strlen(str)) free(str);
	
	st_setType(s, stt_NONE);
}
void sf_flush(State s){st_flushToken(s);}
void sf_flush_reset(State s){
	sf_flush(s);
	sf_reset(s); //flush also resets: token is done and has no consequence.
}

///fall through
void sf_flush_fnp(State s)  {
	///MUST NOT BE CALLED FROM stt_NONE branch! [infinite loop]
	puts("RECUR");
	sf_flush_reset(s); //is done, flushes; searches with NONE
	st_updateFnp(s); //recursive (not really, but in a sense)
	st_Fn(s); 	//call now, for it would skip a char otherwise
				//for this Fn is called instead
}

FnPack with_flush(StepFn sf){return (FnPack){sf, sf_flush};}
FnPack with_flush_reset(StepFn sf){return (FnPack){sf, sf_flush_reset};}
FnPack with_id(StepFn sf){return (FnPack){sf, sf_id};}

FnPack fnp_reset = {sf_reset, sf_id};
FnPack fnp_flush_recur = {sf_flush_fnp, sf_id};
//FnPack fnp_id = {sf_id, sf_id}; //already exists [in header]

/// -- ACTUAL Fns ARE FROM NOW ON

 
#define F_SET_TOKEN(TKN) void sf_set_##TKN(State s){st_crtToken(s, ptt_##TKN);}
///[](){}
F_SET_TOKEN(OP) F_SET_TOKEN(OB) F_SET_TOKEN(OC) //with_flush_reset
F_SET_TOKEN(CP) F_SET_TOKEN(CB) F_SET_TOKEN(CC)

///STR (start, end)
void sf_str_start(State s){ //on " encounter //w flush
	st_setType(s, stt_STR); 
	st_crtToken(s, ptt_STRO);
}
void sf_str_end(State s){ //on _"_ encounter in stt_STR (not stt_STRESC) //w flush
	st_setType(s, stt_NONE); 
	st_crtToken(s, ptt_STRC);
}
//--str helpers
void sf_str_reset(State s){ //no longer continues
	char* str = uc_toStr(st_getChar(s));
	printf("sf_str_reset(%d, %s)\n", st_getType(s), str);
	if (strlen(str)) free(str);
	
	st_setType(s, stt_STR);
}
void sf_str_flush_reset(State s){
	sf_flush(s);
	sf_str_reset(s);
}
void sf_str_flush_fnp(State s){ //no flush
	///MUST NOT BE CALLED FROM stt_STR branch! [infinite loop]
	///just like sf_flush_fnp[recur], but returns to STR
	puts("RECUR STR");
	sf_str_flush_reset(s); //is done, flushes; searches with STR
	st_updateFnp(s); //recursive (not really, but in a sense)
	st_Fn(s); 	//call now, for it would skip a char otherwise
				//for this Fn is called instead
}
void sf_str_err_fnp(State s){ //no flush
	///just add error and recur normaly
	puts("ESC ERR");
	st_setTokenErr(s);
	sf_str_flush_fnp(s);
}
FnPack fnp_str_err_recur = {sf_str_err_fnp, sf_id};
FnPack fnp_str_flush_recur = {sf_str_flush_fnp, sf_id};
///STRD __ 
void sf_strd_start(State s){//no flush 
	st_setType(s, stt_STRD);
	st_putcrtBuffToken(s, ptt_STRD);
}
void sf_strd_step(State s){st_tknputc(s);}//no flush
void sf2_strd_step(State s){//instead of flush
	puts("sf2_strd_step_0");
	if (st_matchChar(s, "\"\\")) sf_flush(s); //on strd leave
	puts("sf2_strd_step_1");
}
FnPack fnp_strd_step = {sf_strd_step, sf2_strd_step};
FnPack fnp_strd_start = {sf_strd_start, sf2_strd_step};
///STRESC __ improve! now works as str|\esc|\str ... sortof [requires \ after itself]
void sf_stresc_start(State s){//no flush
	st_setType(s, stt_STRESC);
	st_crtBuffToken(s, ptt_STRESC); //empty, just data later
}
void sf_stresc_one(State s){//w flush, no reset
	st_setType(s, stt_STR);
	st_tknputc(s);
}
void sf_stresc_step(State s){st_tknputc(s);}//no flush
void sf2_stresc_step(State s){//instead of flush
	if (st_matchChar(s, "\"\\")) {
		sf_flush(s);
		st_setType(s, stt_STR);//??
		if (st_cmpChar(s, '\\')) st_crtBuffToken(s, ptt_STRD);
	} //on strd leave
}
FnPack fnp_stresc_step = {sf_stresc_step, sf2_stresc_step};

void sf_just_minus(State s){st_setType(s, stt_MINUS);}//no flush

///NUM

/*
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

void sf_num_start(State s){//no flush
	st_setType(s, stt_NUM);
	st_putcrtBuffToken(s, ptt_NUMI);
}
void sf_minus_num_start(State s){//no flush
	st_setType(s, stt_NUM);
	st_crtBuffToken(s, ptt_NUMI);
	st_tknaddc(s, '-');
	st_tknputc(s);
}
void sf_num_step(State s){st_tknputc(s);}//no flush
//--
void sf_numD(State s){st_setTokenType(s, ptt_NUMD);}  //w flush reset
void sf_numB(State s){st_setTokenType(s, ptt_NUMB);}  //w flush reset
void sf_numL(State s){st_setTokenType(s, ptt_NUML);}  //w flush reset
void sf_numUL(State s){st_setTokenType(s, ptt_NUMUL);}//w flush reset
//--
void sf_numR(State s){//w id
	st_setType(s, stt_NUMR);
	st_setTokenType(s, ptt_NUMR);
	sf_num_step(s); //or other, global delimiter for multiple args tokens
}
void sf_numF(State s){//w id
	st_setType(s, stt_NUMF);
	st_setTokenType(s, ptt_NUMF);
	sf_num_step(s); //or other, global delimiter for multiple args tokens
}
//--
void sf_numU(State s){//w id
	st_setType(s, stt_NUMU);
	st_setTokenType(s, ptt_NUMUI);
}

///SYMBOL
void sf_symbol_start(State s){//step_flush: if len=1
	puts("sf_symbol_start");
	st_setType(s, stt_SYMBOL);
	st_putcrtBuffToken(s, ptt_SYMBOL);
}
void sf_minus_symbol_start(State s){//no flush
	puts("sf_minus_symbol_start");
	st_setType(s, stt_SYMBOL);
	st_crtBuffToken(s, ptt_SYMBOL);
	st_tknaddc(s, '-');
	st_tknputc(s);
}
void sf_minus_symbol_only(State s){//no flush
	puts("sf_minus_symbol_only");
	st_crtBuffToken(s, ptt_SYMBOL);
	st_tknaddc(s, '-');
	sf_flush_fnp(s); //recur: actually apply curc, is not part of symbol
}
void sf_symbol_step(State s)  {st_tknputc(s);}//no flush

///ID  identifiers
void sf_id_start(State s){//no flush
	puts("sf_id_start");
	st_setType(s, stt_ID);
	st_putcrtBuffToken(s, ptt_ID);
}
void sf_id_step(State s)  {st_tknputc(s);}//no flush

///OPEN [opener characteres like : # ...]
void sf_open(State s){st_putcrtBuffToken(s, ptt_OPEN);} //with flush

///FUNCTION "\", "\\"
void sf_fnl(State s) {
	st_setType(s, stt_FN);
	st_crtToken(s, ptt_FNL);
}
void sf_fnn(State s) {st_setTokenType(s, ptt_FNN);}

///COMMENT [ ; ]
void sf_cmnt_start(State s){//w id, uses universal end
	st_setType(s, stt_CMNT);
	st_crtToken(s, ptt_CMNT);
}





//</all fns>

FnPack triforce_find(Stt tt, UChar c){ //+num, symbol, id
	if(is_num_first(c)) return with_id(sf_num_start); //NUM
	if(is_symbol(c)) return with_id(sf_symbol_start);   //SYMBOL
	if(is_id_first(c)) return with_id(sf_id_start); //ID
	puts("triforce through"); ///minimize this [TODO solve unicode IDs here]
	return fnp_id;
}

FnPack minus_find(Stt tt, UChar c){ //-num, symbol
	puts("minus find NUM");
	if(is_num_first(c)) return with_id(sf_minus_num_start); //NUM
	puts("minus find SYMBOL");
	if(is_symbol(c)) return with_id(sf_minus_symbol_start); //SYMBOL (starts with -)
	return with_id(sf_minus_symbol_only); //SYBMOL: just -
}
 
FnPack fnp_find(Stt tt, UChar c){			///main enetery

	char* str = uc_toStr(c);
	printf("fnp_find(%d, %s)\n", tt, str[0] == '\n' ? "ENTER" : str); //DEBUG
	if (strlen(str)) free(str);
	
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
		
		case stt_MINUS: return minus_find(tt, c);
		case stt_ID: return is_id(c) 
				? with_id(sf_id_step)
				: fnp_flush_recur;  break;
		case stt_SYMBOL: return is_symbol(c) 
				? with_id(sf_symbol_step)
				: fnp_flush_recur; break;
		case stt_NUM: switch(c) {
			case '.': return with_id(sf_numF);
			case '/': return with_id(sf_numR);
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
			case '#': return with_flush_reset(sf_open);
			case ' ':
			case ',':
			case '\t':
			case '\n':return fnp_id; //just ignore, might be used in future...
			default : return triforce_find(tt, c); //no need to check for Opens
		} break;
		
		
		default: return fnp_id; //something weird; for now not important
	}
}

