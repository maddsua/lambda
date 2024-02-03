
#	single file include generator
TOOL_GENERATEINCLUDE_TARGET = $(EXEPFX)generateinclude$(EXEEXT)
tool.generateinclude: $(TOOL_GENERATEINCLUDE_TARGET)

$(TOOL_GENERATEINCLUDE_TARGET): tools/headers/generateinclude.cpp
	g++ -std=c++20 -s tools/headers/generateinclude.cpp -o $(TOOL_GENERATEINCLUDE_TARGET)
