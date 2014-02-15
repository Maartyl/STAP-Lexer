//dependency problems
#ifndef TYPES_HEADER
#define TYPES_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str_buffer.h"
#include "uchar.h"

 
//<Cascade token types>
typedef enum eStreamTokenType{ //stream states
	stt_NONE, stt_STR, stt_STRD, stt_STRESC,
	stt_MINUS, stt_ID, stt_SYMBOL, stt_OPEN,
	stt_NUM, stt_NUMF, stt_NUMD, stt_NUMR, stt_NUMU,
	stt_FN,
	stt_CMNT
} Stt;

typedef enum ePublicTokenType{ //any type to be used in tokens themselves
	 ptt_OP = 111 // ( //Parenthses
	,ptt_CP = 112 // )
	,ptt_OB = 121 // [ //Brackets
	,ptt_CB = 122 // ]
	,ptt_OC = 131 // { //Curly
	,ptt_CC = 132 // } 
	,ptt_STRO = 10 // _"_ (start)  //Open
	,ptt_STRC = 11 // _"_ (end)	   //Close
	,ptt_STRD = 15 //actual part of string, that is Data
	,ptt_STRESC = 16
	,ptt_NUMI = 201	 //signed [int, long]
	,ptt_NUML = 202
	,ptt_NUMB = 205	 //unsigned [byte, uint, ulong]
	,ptt_NUMUI = 206 //can have (-) TODO: ERR / IGNORE
	,ptt_NUMUL = 207
	,ptt_NUMF = 211	 //[float, double] //1. == 1.0 , 5.8D ...
	,ptt_NUMD = 212
	,ptt_NUMR = 221	 //[rational i.e.  5/8]
	,ptt_CMNT = 400	//comment
	,ptt_FNL = 51	//function lambda
	,ptt_FNN = 52	//function named
	,ptt_OPEN = 6   //special starts (:, @, ...)
	,ptt_ID = 1     //normal identifier
	,ptt_SYMBOL = 2 //symbol identifier
	,ptt_EOF = -1
	,ptt_ERR = 4096 //combinable
} Ptt;


//</Cascade token types>

//<Lexer>
/* flux: keps track of where I am 
   just so everything is not named "position"...
   snapshot will be stord in tokens*/
typedef struct sFlux {
	int pos, col, row; /*start: 0, 1, 1*/
} Flux;

//Token structure; to be passed around when building
typedef struct sToken {
	Flux   flux;
	Ptt    type;
	Buffer payload;
} Token;
//</Lexer>

//<State forward>
typedef struct sState * State;
//</State>

//<Cascade>
typedef void (*StepFn)(State s); //fn: can change state; nothing much else

typedef struct sFnPack {
	StepFn fn, fn2; //fn: cur char; fn2: next char
} FnPack;
//</Cascade>

//<State>
typedef struct sState { //not necessarily consistent!
	FILE *fin, *fout;//in, out streams
	Stt tt; 	//(stream)tokenType - state
	UChar curc;	//current loaded character to work with
	Flux flux;	//position in source file
	FnPack fp;	//fns to manipulate state by current char
	Token t;	//currently being processed token (buffer, not type)
} sState; 
//</State>

//<Consts>
#define SEPAR 30 /*RS*/
//</Consts>

#endif
