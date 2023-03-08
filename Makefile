
DEV_APP   = lambda.exe
DEV_MAIN  = dev
LIBNAME   = libmaddsualambda
LIBDIR    = lib
FLAGS     = -std=c++20
OBJECTS   = src/sockets.o src/http.o src/lambda.o src/statuscode.o src/mimetypes.o src/fetch.o src/compress.o src/filesystem.o src/base64.o
LINK_LIBS = -lws2_32 -lz -lbrotli_static

.PHONY: all all-before all-after clean clean-custom run-custom

all: all-before $(DEV_APP) all-after

clean: clean-custom
	del /S *.o *.exe

run: run-custom
	$(DEV_APP)



## Dev executable

$(DEV_APP): $(DEV_MAIN).o $(OBJECTS)
	g++ $(DEV_MAIN).o $(OBJECTS) -o $(DEV_APP) $(LINK_LIBS)

$(DEV_MAIN).o: $(DEV_MAIN).cpp
	g++ -c $(DEV_MAIN).cpp -o $(DEV_MAIN).o $(FLAGS)



## Library Build

lib: $(OBJECTS)
	ar rvs $(LIBDIR)/$(LIBNAME).a $(OBJECTS)



## Library objects

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
