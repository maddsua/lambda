
TOOL_UPDATEDLLINFO_TARGET = $(EXEPFX)updatedllinfo$(EXEEXT)
tool.updatedllinfo: $(TOOL_UPDATEDLLINFO_TARGET)

$(TOOL_UPDATEDLLINFO_TARGET): tools/updatedllinfo.cpp
	g++ -std=c++20 -s tools/updatedllinfo.cpp -o $(TOOL_UPDATEDLLINFO_TARGET)


TOOL_GENERATEINCLUDE_TARGET = $(EXEPFX)generateinclude$(EXEEXT)
tool.generateinclude: $(TOOL_GENERATEINCLUDE_TARGET)

$(TOOL_GENERATEINCLUDE_TARGET): tools/generateinclude.cpp $(LIB_CORE_POLYFILL)
	g++ -std=c++20 -s tools/generateinclude.cpp $(LIB_CORE_POLYFILL) -o $(TOOL_GENERATEINCLUDE_TARGET)
