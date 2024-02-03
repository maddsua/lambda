
#	single file include generator
TOOL_GENERATEINCLUDE_TARGET = $(EXEPFX)generateinclude$(EXEEXT)
tool.generateinclude: $(TOOL_GENERATEINCLUDE_TARGET)

$(TOOL_GENERATEINCLUDE_TARGET): tools/generateinclude/main.cpp
	g++ -std=c++20 -s tools/generateinclude/main.cpp -o $(TOOL_GENERATEINCLUDE_TARGET)
