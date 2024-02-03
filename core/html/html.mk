
LIB_CORE_HTML			=	core/html.a
LIB_CORE_HTML_TEMPLATES	=	core/html/resources/servicepage.res
LIB_CORE_HTML_DEPS		=	core/html/templates.o core/html/engine.o $(LIB_CORE_HTML_TEMPLATES)

# html templates and stuff
$(LIB_CORE_HTML): $(LIB_CORE_HTML_DEPS)
	ar rvs $(LIB_CORE_HTML) $(LIB_CORE_HTML_DEPS)

core/html/templates.o: core/html/templates.cpp
	g++ -c $(CFLAGS) core/html/templates.cpp -o core/html/templates.o

core/html/engine.o: core/html/engine.cpp
	g++ -c $(CFLAGS) core/html/engine.cpp -o core/html/engine.o

core/html/resources/servicepage.res: core/html/resources/servicepage.html
	objcopy --input-target binary --output-target $(BINRES_TARGET) --binary-architecture i386:x86-64 core/html/resources/servicepage.html core/html/resources/servicepage.res
