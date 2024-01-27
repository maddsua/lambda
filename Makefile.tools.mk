
updatedllinfo: updatedllinfo$(EXEEXT)

updatedllinfo$(EXEEXT): tools/updatedllinfo.cpp
	g++ -std=c++20 -s tools/updatedllinfo.cpp -o updatedllinfo$(EXEEXT)


generateinclude: generateinclude$(EXEEXT)

generateinclude$(EXEEXT): tools/generateinclude.cpp
	g++ -std=c++20 -s tools/generateinclude.cpp -o generateinclude$(EXEEXT)
