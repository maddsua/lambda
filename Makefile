#	-static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

APP_DEV    = lambda.exe
APP_DEMO   = demo/lambda.exe
LIBNAME    = mdslambda

OBJECTS    = src/sockets.o src/http.o src/lambda.o src/statuscode.o src/mimetypes.o src/fetch.o src/compression.o src/filesystem.o src/base64.o src/hex.o src/generate.o src/sha2.o src/sha1.o 

FLAGS      = -std=c++20
LIBS       = -lws2_32 -lz -lbrotlicommon -lbrotlidec -lbrotlienc -lwinmm

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
	g++ $(OBJECTS) main.o -o $(APP_DEV) $(LIBS) $(FLAGS)

main.o: main.cpp
	g++ -c main.cpp -o main.o $(FLAGS)


# ----
#	demo app
# ----
demo: demo/main.o
#	static link
	g++ demo/main.o -o $(APP_DEMO) -L. -l:lib$(LIBNAME).a $(LIBS) $(FLAGS)
#	dynamic link
#	g++ demo/main.o -o $(APP_DEMO) -L. -l$(LIBNAME) $(LIBS) $(FLAGS)

demo/main.o: demo/main.cpp
	g++ -c demo/main.cpp -o demo/main.o $(FLAGS)


# ----
#	lib
# ----
lib: $(OBJECTS)
#	make static lib
	ar rvs lib$(LIBNAME).a $(OBJECTS)
#	make dll
	g++ $(OBJECTS) -s -shared -o $(LIBNAME).dll -Wl,--out-implib,lib$(LIBNAME).dll.a $(LIBS) $(FLAGS)

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

src/hex.o: src/hex.cpp
	g++ -c src/hex.cpp -o src/hex.o $(FLAGS)

src/generate.o: src/generate.cpp
	g++ -c src/generate.cpp -o src/generate.o $(FLAGS)

src/sha2.o: src/sha2.cpp
	g++ -c src/sha2.cpp -o src/sha2.o $(FLAGS)

src/sha1.o: src/sha1.cpp
	g++ -c src/sha1.cpp -o src/sha1.o $(FLAGS)