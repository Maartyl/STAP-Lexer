#include "uchar.h"


UChar uc_read(FILE *stream){
	int nc(){return fgetc(stream);} //next char
	
	/*	00000000 -- 0000007F: 	0xxxxxxx
		00000080 -- 000007FF: 	110xxxxx 10xxxxxx
		00000800 -- 0000FFFF: 	1110xxxx 10xxxxxx 10xxxxxx
		00010000 -- 001FFFFF: 	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	*/
	//dealing with UTF:
	
	int a = nc(); //get first byte
	UChar acc = a; //accumulator
	
	if (a>=128) { //simple check for most of the time, [-1 works too]
		//1<<x == 2^x
		//0x3F: masks last 6 bits, others are similar
		///idea:
		///take bits in byte, multiply by how much is left to end
		///repeat for each byte, and add together
			//acc = (0x1F&a)*(1<<6)+(0x3F&b);
		///this could be represented as simple moving of bits :
		///shift bits of appropriate byte of ho how much is left to end, and combine
			//acc = ((0x1F&a)<<6)|(0x3F&b);
		
		///TODO: add checking [not -1, starts with b10...]
		
		if((a&0xE0)==0xC0){ //match part of int
		//2B
			int b = nc();
			//decompose according to table above
			//acc = (0x1F&a)*(1<<6)+(0x3F&b); 
			acc = ((0x1F&a)<<6)|(0x3F&b);
		}else if((a&0xF0)==0xE0){ 
		//3B
			int b = nc();
			int c = nc();
			acc = 	 ((0x0F&a)<<12)
					|((0x3F&b)<<6)
					| (0x3F&c);
		}else if((a&0xF8)==0xF0){ 
		//4B
			int b = nc(); //I'm not sure about preserving order otherwise...
			int c = nc();
			int d = nc();
			acc = 	 ((0x07&a)<<18)
					|((0x3F&b)<<12)
					|((0x3F&c)<<6)
					| (0x3F&d);
		}
	}

	return acc;
}


int uc_write(UChar c, FILE *stream){
	void l(int a){fputc(a, stream);}
	return uc_decompose(c, l);
}


int uc_decompose(UChar c, void(*f)(int)){
	if (c<0x80) { //valid ASCII value, 1B
		(*f)(c);
		return 1;
	} else if (c<0x800){//2B
		(*f)(0xC0|(0x1F&(c>>6))); //110 00000|00000xxx.xx000000
		(*f)(0x80|(0x3F&c));      //10 000000|00000000.00xxxxxx
		return 2;
	} else if (c<0x10000){//3B
		(*f)(0xE0|(0x0F&(c>>12)));//1110 0000|00000000.xxxx0000.00000000
		(*f)(0x80|(0x3F&(c>>6))); //10 000000|00000000.0000xxxx.xx000000
		(*f)(0x80|(0x3F&c));      //10 000000|00000000.00000000.00xxxxxx
		return 3;
	} else if (c<0x200000){//4B
		(*f)(0xE0|(0x07&(c>>18)));//11110 000|000xxx00.00000000.00000000
		(*f)(0x80|(0x3F&(c>>12)));//10 000000|000000xx.xxxx0000.00000000
		(*f)(0x80|(0x3F&(c>>6))); //10 000000|00000000.0000xxxx.xx000000
		(*f)(0x80|(0x3F&c));      //10 000000|00000000.00000000.00xxxxxx
		return 4;
	} else return 0; //too large number, invalid [negatives are valid invalids]
}

char *uc_toStr(UChar c){
	int arr[4]; //to store yielded result, integer values are under 256
	int i = 0;  //index to arr; becomes length (one after last)
	void l(int a){arr[i++] = a;}
	if (uc_decompose(c, l)) {
		char* p = malloc(sizeof(char)*(i+1)); //+1: i==len + null
		for (int j=0; j<i; j++) {//for all valid chars in arr
			p[j] = (char) arr[j];
		}
		p[i] = '\0'; //null terminated
		return p;
	} else return "";
}












