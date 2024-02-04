
LIB_CORE_PAGES			=	core/pages.a
LIB_CORE_PAGES_PAGES	=	core/pages/resources/servicepage.res
LIB_CORE_PAGES_DEPS		=	core/pages/resourcemap.o core/pages/renderer.o core/pages/errorpage.o $(LIB_CORE_PAGES_PAGES)

# pages stuff
$(LIB_CORE_PAGES): $(LIB_CORE_PAGES_DEPS)
	ar rvs $(LIB_CORE_PAGES) $(LIB_CORE_PAGES_DEPS)

core/pages/resourcemap.o: core/pages/resourcemap.cpp
	g++ -c $(CFLAGS) core/pages/resourcemap.cpp -o core/pages/resourcemap.o

core/pages/renderer.o: core/pages/renderer.cpp
	g++ -c $(CFLAGS) core/pages/renderer.cpp -o core/pages/renderer.o

core/pages/errorpage.o: core/pages/errorpage.cpp
	g++ -c $(CFLAGS) core/pages/errorpage.cpp -o core/pages/errorpage.o

core/pages/resources/servicepage.res: core/pages/resources/servicepage.html
	objcopy --input-target binary --output-target $(BINRES_TARGET) --binary-architecture i386:x86-64 core/pages/resources/servicepage.html core/pages/resources/servicepage.res
