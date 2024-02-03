
#	single file include generator
TOOL_GENERATEINCLUDE_TARGET = $(EXEPFX)generateinclude$(EXEEXT)
tool.generateinclude: $(TOOL_GENERATEINCLUDE_TARGET)

TOOL_GENERATEINCLUDE_DEPS = tools/generateinclude/main.o tools/generateinclude/headerfile.res

$(TOOL_GENERATEINCLUDE_TARGET): $(TOOL_GENERATEINCLUDE_DEPS)
	g++ -std=c++20 -s $(TOOL_GENERATEINCLUDE_DEPS) -o $(TOOL_GENERATEINCLUDE_TARGET)

tools/generateinclude/main.o: tools/generateinclude/main.cpp
	g++ -c -std=c++20 tools/generateinclude/main.cpp -o tools/generateinclude/main.o

tools/generateinclude/headerfile.res: tools/generateinclude/headerfile.txt
	objcopy --input-target binary --output-target $(BINRES_TARGET) --binary-architecture i386:x86-64 tools/generateinclude/headerfile.txt tools/generateinclude/headerfile.res
