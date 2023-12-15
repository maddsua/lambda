
updatedllinfo: updatedllinfo.exe

updatedllinfo.exe: tools/updatedllinfo.cpp
	g++ tools/updatedllinfo.cpp -o updatedllinfo.exe
