# 2023 maddsua's lambda
# Demo/Test app
# makefile for windows build
# GCC-12.2-ucrt64 is in use

APP_DEV		= lambda.exe
LIBNAME		= lambda

OBJECTS		= src/constants.o src/util.o src/httpcore.o src/lambda.o src/fetch.o src/compress.o src/filesystem.o src/sha.o src/localdb.o

FLAGS		= -std=c++20 -g
LIBS_SHARED	= -lz -lbrotlicommon -lbrotlidec -lbrotlienc

#	to get these static libs you need to compile brotli and zlib youself
#	it's basically just compiling all source files to objects
#	and then putting all of them into .a static lib
LIBS_STATIC	= -l:libz.a -l:libbrotli.a
LIBS_SYSTEM	= -lws2_32 -lwinmm

LIBSTATIC	= lib$(LIBNAME).a
LIBSHARED	= $(LIBNAME).dll

#	-static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

.PHONY: all all-before all-after action-custom
all: all-before $(APP_DEV) all-after

purge: action-custom
	del /S *.o *.exe *.a *.dll *.res
#	rm -rf *.o *.exe *.a *.dll *.res

run: action-custom
	$(APP_DEV)


# ----
#	labmda demo/test app
# ----
#	regular dev app
$(APP_DEV): main.o $(OBJECTS)
	g++ main.o $(OBJECTS) $(LIBS_SHARED) $(LIBS_SYSTEM) $(FLAGS) -o $(APP_DEV)

#	fully static build, version for the demo
static: $(LIBSTATIC) main.o
	g++ -static main.o -L. -l$(LIBNAME) $(LIBS_STATIC) $(LIBS_SYSTEM) $(FLAGS) -o $(APP_DEV)

#	dynamically linked to all the dlls
dynamic: libshared main.o
	g++ main.o -L. -l$(LIBNAME) $(FLAGS) -o $(APP_DEV)


# ----
#	dev main
# ----
main.o: main.cpp
	g++ -c main.cpp -o main.o $(FLAGS)


# ----
#	lib
# ----
#	make static lib
libstatic: $(LIBSTATIC)

$(LIBSTATIC): $(OBJECTS)
	ar rvs $(LIBSTATIC) $(OBJECTS)

#	make dll
libshared: $(LIBSHARED)

$(LIBSHARED): $(OBJECTS) $(LIBNAME).res
	g++ $(OBJECTS) $(LIBNAME).res $(LIBS_SHARED) $(LIBS_SYSTEM) $(FLAGS) -s -shared -o $(LIBSHARED) -Wl,--out-implib,lib$(LIBSHARED).a


# ----
#	resources
# ----
$(LIBNAME).res: $(LIBNAME).rc
	windres -i $(LIBNAME).rc --input-format=rc -o $(LIBNAME).res -O coff 


# ----
#	main components
# ----
src/lambda.o: src/lambda.cpp
	g++ -c src/lambda.cpp -o src/lambda.o $(FLAGS)

src/httpcore.o: src/httpcore.cpp
	g++ -c src/httpcore.cpp -o src/httpcore.o $(FLAGS)

src/constants.o: src/constants.cpp
	g++ -c src/constants.cpp -o src/constants.o $(FLAGS)

src/http.o: src/http.cpp
	g++ -c src/http.cpp -o src/http.o $(FLAGS)

src/mimetypes.o: src/mimetypes.cpp
	g++ -c src/mimetypes.cpp -o src/mimetypes.o $(FLAGS)

src/fetch.o: src/fetch.cpp
	g++ -c src/fetch.cpp -o src/fetch.o $(FLAGS)

src/compress.o: src/compress.cpp
	g++ -c src/compress.cpp -o src/compress.o $(FLAGS)

src/filesystem.o: src/filesystem.cpp
	g++ -c src/filesystem.cpp -o src/filesystem.o $(FLAGS)

src/base64.o: src/base64.cpp
	g++ -c src/base64.cpp -o src/base64.o $(FLAGS)

src/util.o: src/util.cpp
	g++ -c src/util.cpp -o src/util.o $(FLAGS)

src/sha.o: src/sha.cpp
	g++ -c src/sha.cpp -o src/sha.o $(FLAGS)


# ----
#	kinda plugins
# ----
src/localdb.o: src/localdb.cpp
	g++ -c src/localdb.cpp -o src/localdb.o $(FLAGS)