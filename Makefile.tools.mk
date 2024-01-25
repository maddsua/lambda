
updatedllinfo: updatedllinfo$(EXEEXT)

updatedllinfo$(EXEEXT): tools/updatedllinfo.cpp
	g++ -std=c++20 -s tools/updatedllinfo.cpp -o updatedllinfo$(EXEEXT)
