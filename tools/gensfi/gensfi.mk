
#	single file include generator
TOOL_GENSFI_TARGET = $(EXEPFX)gensfitool$(EXEEXT)
tool.gensfi: $(TOOL_GENSFI_TARGET)

$(TOOL_GENSFI_TARGET): tools/gensfi/main.cpp
	g++ -std=c++20 -s tools/gensfi/main.cpp -o $(TOOL_GENSFI_TARGET)
