


all:
	g++ -o main.exe main.cpp -Isrc/include -Lsrc/lib -lSDL3

debug:
	g++ -o main.exe main.cpp -Isrc/include -Lsrc/lib -lSDL3 -DDEBUG



clean:
	rm main.exe
	rm main.o 
	