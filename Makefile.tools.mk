
TOOL_UPDATEDLLINFO_TARGET = $(TEMPBIN)updatedllinfo$(EXEEXT)
tool.updatedllinfo: $(TOOL_UPDATEDLLINFO_TARGET)

$(TOOL_UPDATEDLLINFO_TARGET): tools/updatedllinfo.cpp
	g++ -std=c++20 -s tools/updatedllinfo.cpp -o $(TOOL_UPDATEDLLINFO_TARGET)


TOOL_GENERATEINCLUDE_TARGET = $(TEMPBIN)generateinclude$(EXEEXT)
tool.generateinclude: $(TOOL_GENERATEINCLUDE_TARGET)

$(TOOL_GENERATEINCLUDE_TARGET): tools/generateinclude.cpp
	g++ -std=c++20 -s tools/generateinclude.cpp -o $(TOOL_GENERATEINCLUDE_TARGET)
