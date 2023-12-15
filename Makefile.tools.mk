
updatedllinfo: updatedllinfo.exe

updatedllinfo.exe: tools/updatedllinfo.cpp
	g++ $(CFLAGS) tools/updatedllinfo.cpp -o updatedllinfo.exe
