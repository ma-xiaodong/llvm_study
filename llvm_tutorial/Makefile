LLVM_INC = -I/usr/include/llvm-6.0 -I/usr/include/llvm-c-6.0
LIBS = `llvm-config-6.0 --libs`

parser_c: ./src/parser_c.cc ./src/token.cc ./include/token.h ./include/parser_c.h
	g++ -g -O0 -c ./src/token.cc -o ./build/token.o
	g++ -g -O0 ./src/parser_c.cc ./build/token.o -o ./build/parser_c

parser_llvm: ./src/parser_c.cc ./src/token.cc ./include/token.h ./include/parser_c.h
	clang++ -g -O0 -c ./src/token.cc -o ./build/token.o
	clang++ ${LLVM_INC} -O0 ./src/parser_llvm.cc ./build/token.o -o ./build/parser_llvm ${LIBS}
