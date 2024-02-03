
# "update dll info" tool
TOOL_UPDATEDLLINFO_TARGET = $(EXEPFX)updatedllinfo$(EXEEXT)
tool.updatedllinfo: $(TOOL_UPDATEDLLINFO_TARGET)

$(TOOL_UPDATEDLLINFO_TARGET): tools/updatedllinfo/main.cpp
	g++ -std=c++20 -s tools/updatedllinfo/main.cpp -o $(TOOL_UPDATEDLLINFO_TARGET)
