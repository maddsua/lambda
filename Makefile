#	-static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

APP_DEV    = lambda.exe
APP_DEMO   = lambda-server.exe
LIBNAME    = mdslambda

OBJECTS    = src/sockets.o src/http.o src/lambda.o src/statuscode.o src/mimetypes.o src/fetch.o src/compression.o src/filesystem.o src/base64.o src/util.o src/sha.o

FLAGS      = -std=c++20
LIBS       = -lz -lbrotlicommon -lbrotlidec -lbrotlienc
LIB_STC    = -l:libz.a -l:libbrotli.a
LIBS_SYS   = -lws2_32 -lwinmm

.PHONY: all all-before all-after clean-custom run-custom lib demo
all: all-before $(APP_DEV) all-after

clean: clean-custom
	del /S *.exe *.a *.dll
#	rm -rf *.exe *.a *.dll

purge: clean-custom
	del /S *.o *.exe *.a *.dll
#	rm -rf *.o *.exe *.a *.dll

run: run-custom
	$(APP_DEV)


# ----
#	dev app
# ----
$(APP_DEV): $(OBJECTS) main.o
	g++ $(OBJECTS) main.o $(LIBS_SYS) $(LIBS) $(FLAGS) -o $(APP_DEV)

main.o: main.cpp
	g++ -c main.cpp -o main.o $(FLAGS)


# ----
#	demo app
# ----
demo: main.o
#	static link
#	g++ demo/main.o -L. -l:lib$(LIBNAME).a $(LIBS) $(FLAGS) -o $(APP_DEMO)
#	dynamic link
	g++ main.o -L. -l$(LIBNAME) $(FLAGS) -o $(APP_DEMO)


# ----
#	lib
# ----
#	make static lib
libstatic: $(OBJECTS)
	ar rvs lib$(LIBNAME).a $(OBJECTS)

#	make dll
libshared: $(OBJECTS)
	g++ $(OBJECTS) $(LIBS_SYS) $(LIB_STC) $(FLAGS) -s -shared -o $(LIBNAME).dll -Wl,--out-implib,lib$(LIBNAME).dll.a

# ----
#	lib objects
# ----
src/lambda.o: src/lambda.cpp
	g++ -c src/lambda.cpp -o src/lambda.o $(FLAGS)

src/sockets.o: src/sockets.cpp
	g++ -c src/sockets.cpp -o src/sockets.o $(FLAGS)

src/http.o: src/http.cpp
	g++ -c src/http.cpp -o src/http.o $(FLAGS)

src/statuscode.o: src/statuscode.cpp
	g++ -c src/statuscode.cpp -o src/statuscode.o $(FLAGS)

src/mimetypes.o: src/mimetypes.cpp
	g++ -c src/mimetypes.cpp -o src/mimetypes.o $(FLAGS)

src/fetch.o: src/fetch.cpp
	g++ -c src/fetch.cpp -o src/fetch.o $(FLAGS)

src/compression.o: src/compression.cpp
	g++ -c src/compression.cpp -o src/compression.o $(FLAGS)

src/filesystem.o: src/filesystem.cpp
	g++ -c src/filesystem.cpp -o src/filesystem.o $(FLAGS)

src/base64.o: src/base64.cpp
	g++ -c src/base64.cpp -o src/base64.o $(FLAGS)

src/util.o: src/util.cpp
	g++ -c src/util.cpp -o src/util.o $(FLAGS)

src/sha.o: src/sha.cpp
	g++ -c src/sha.cpp -o src/sha.o $(FLAGS)
