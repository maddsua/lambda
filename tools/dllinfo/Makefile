
# "update dll info" tool
TOOL_UPDATEDLLINFO_TARGET = $(EXEPFX)updatedllinfo$(EXEEXT)
tool.updatedllinfo: $(TOOL_UPDATEDLLINFO_TARGET)

$(TOOL_UPDATEDLLINFO_TARGET): tools/dllinfo/updatedllinfo.cpp
	g++ -std=c++20 -s tools/dllinfo/updatedllinfo.cpp -o $(TOOL_UPDATEDLLINFO_TARGET)
