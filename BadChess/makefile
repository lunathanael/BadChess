SRCS=$(wildcard *.cpp)
NAME=BadChess

all:
	g++ -Ofast $(SRCS) tinycthread.c ./nnue/nnue.cpp ./nnue/misc.cpp -o $(NAME) -pthread -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse
	x86_64-w64-mingw32-gcc -Ofast $(SRCS) tinycthread.c ./nnue/nnue.cpp ./nnue/misc.cpp -o BadChess-64.exe -pthread -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse -lstdc++
