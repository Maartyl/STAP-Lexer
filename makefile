all: src/*.c
	gcc -Wall -std=gnu99 -O3 -o ./bin/stap-lex     ./src/*.c 
