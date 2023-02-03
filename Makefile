
APP_DEV    = lambda.exe
APP_DEMO   = demo/lambda.exe
LIBNAME    = mdslambda

OBJECTS    = src/sockets.o src/http.o src/lambda.o src/statuscode.o src/mimetypes.o src/fetch.o src/compress.o src/filesystem.o src/base64.o
FLAGS      = -std=c++20
LIBS       = -lws2_32 -lz -lbrotlicommon -lbrotlidec -lbrotlienc


.PHONY: all all-before all-after clean-custom run-custom lib demo
all: all-before $(APP_DEV) all-after


clean: clean-custom
	del /S *.o *.exe *.a
#	rm -rf *.o *.exe *.a

run: run-custom
	$(APP_DEV)


# ----
#	dev app
# ----
$(APP_DEV): $(OBJECTS) main.o
	g++ $(OBJECTS) main.o -o $(APP_DEV) $(LIBS)

main.o: main.cpp
	g++ -c main.cpp -o main.o $(FLAGS)


# ----
#	demo app
# ----
demo: $(OBJECTS) demo/main.o
	g++ $(OBJECTS) demo/main.o -o $(APP_DEMO) $(LIBS) $(LIB).a -L"../lib/"

demo/main.o: demo/main.cpp
	g++ -c demo/main.cpp -o demo/main.o $(FLAGS)


# ----
#	lib
# ----
libstatic: $(OBJECTS)
	ar rvs lib$(LIBNAME).a $(OBJECTS)

libshared: $(OBJECTS)
	g++ $(OBJECTS) -s -shared -o $(LIBNAME).dll -Wl,-s,--out-implib,lib$(LIBNAME).dll.a $(LIBS) $(FLAGS)

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

src/compress.o: src/compress.cpp
	g++ -c src/compress.cpp -o src/compress.o $(FLAGS)

src/filesystem.o: src/filesystem.cpp
	g++ -c src/filesystem.cpp -o src/filesystem.o $(FLAGS)

src/base64.o: src/base64.cpp
	g++ -c src/base64.cpp -o src/base64.o $(FLAGS)
