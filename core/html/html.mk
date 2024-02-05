
LIB_CORE_html			=	core/html.a
LIB_CORE_HTML_PAGES		=	core/html/resources/servicepage.res
LIB_CORE_HTML_DEPS		=	core/html/renderer.o core/html/templatemap.o $(LIB_CORE_HTML_PAGES)

# html stuff
$(LIB_CORE_HTML): $(LIB_CORE_HTML_DEPS)
	ar rvs $(LIB_CORE_HTML) $(LIB_CORE_HTML_DEPS)

core/html/renderer.o: core/html/renderer.cpp
	g++ -c $(CFLAGS) core/html/renderer.cpp -o core/html/renderer.o

core/html/templatemap.o: core/html/templatemap.cpp
	g++ -c $(CFLAGS) core/html/templatemap.cpp -o core/html/templatemap.o

core/html/resources/servicepage.res: core/html/resources/servicepage.html
	objcopy --input-target binary --output-target $(BINRES_TARGET) --binary-architecture i386:x86-64 core/html/resources/servicepage.html core/html/resources/servicepage.res
