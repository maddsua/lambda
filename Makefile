
APP      = lambda.exe
MAIN     = main
OBJECTS  = main.o src/lambda.o src/http.o src/statuscode.o src/mimetypes.o src/fetch.o src/bufferCompress.o
FLAGS    = -std=c++20
LIBS     = -lmswsock -lwinmm -lws2_32 -lz

.PHONY: all all-before all-after clean clean-custom run-custom

all: all-before $(APP) all-after

clean: clean-custom
	del /S *.o *.exe

run: run-custom
	$(APP)

$(APP): $(OBJECTS)
	g++ $(OBJECTS) -o $(APP) $(LIBS)

$(MAIN).o: $(MAIN).cpp
	g++ -c $(MAIN).cpp -o $(MAIN).o $(FLAGS)

src/lambda.o: src/lambda.cpp
	g++ -c src/lambda.cpp -o src/lambda.o $(FLAGS)

src/http.o: src/http.cpp
	g++ -c src/http.cpp -o src/http.o $(FLAGS)

src/statuscode.o: src/statuscode.cpp
	g++ -c src/statuscode.cpp -o src/statuscode.o $(FLAGS)

src/mimetypes.o: src/mimetypes.cpp
	g++ -c src/mimetypes.cpp -o src/mimetypes.o $(FLAGS)

src/fetch.o: src/fetch.cpp
	g++ -c src/fetch.cpp -o src/fetch.o $(FLAGS)

src/bufferCompress.o: src/bufferCompress.cpp
	g++ -c src/bufferCompress.cpp -o src/bufferCompress.o $(FLAGS)

