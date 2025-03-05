INC_DIR=-I/usr/include/llvm-c-10/
INC_DIR+=-I/usr/include/llvm-10/
LIBS=`llvm-config-10 --libs`

exam0: exam0.cpp
	clang++ -g -std=c++14 ${INC_DIR} exam0.cpp -o ../build/exam0 ${LIBS} -lpthread -lncurses
