all: postprocess.exe

postprocess.exe: data.o postprocess.o
	i686-w64-mingw32-g++ data.o postprocess.o -o postprocess.exe -ld3dx9 -ld3d9 -ldxguid

data.o: data.cpp data.h
	i686-w64-mingw32-g++ data.cpp -c

postprocess.o: postprocess.cpp data.h
	i686-w64-mingw32-g++ postprocess.cpp -c

clean:
	rm -f data.o postprocess.o postprocess.exe
