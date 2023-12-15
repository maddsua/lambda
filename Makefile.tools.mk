
updatedllinfo: updatedllinfo.exe

updatedllinfo.exe: tools/updatedllinfo.cpp
	g++ -std=c++20 -s tools/updatedllinfo.cpp -o updatedllinfo.exe
