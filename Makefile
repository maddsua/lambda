# 2023 maddsua's lambda

APP_DEV    = lambda.exe
APP_DEMO   = lambda-server.exe
LIBNAME    = lambda

OBJECTS    = src/sockets.o src/http.o src/lambda.o src/mimetypes.o src/fetch.o src/compression.o src/filesystem.o src/base64.o src/util.o src/sha.o src/radishdb.o

FLAGS      = -std=c++20
LIBS_SYS   = -lws2_32 -lwinmm
LIBS       = $(LIBS_SYS) -lz -lbrotlicommon -lbrotlidec -lbrotlienc

#	-static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread -Wl,-Bdynamic

.PHONY: all all-before all-after action-custom
all: all-before $(APP_DEV) all-after

clean: action-custom
	del /S *.exe *.a *.dll
#	rm -rf *.exe *.a *.dll

purge: action-custom
	del /S *.o *.exe *.a *.dll
#	rm -rf *.o *.exe *.a *.dll

run: action-custom
	$(APP_DEV)


# ----
#	dev app
# ----
$(APP_DEV): $(OBJECTS) main.o
	g++ $(OBJECTS) main.o $(LIBS) $(FLAGS) -o $(APP_DEV)


main.o: main.cpp
	g++ -c main.cpp -o main.o $(FLAGS)


# ----
#	demo app
# ----
demo: main.o
#	dynamic linking
	g++ main.o -L. -l$(LIBNAME) $(FLAGS) -o $(APP_DEMO)
#	static linking
#	g++ demo/main.o -L. -l:lib$(LIBNAME).a $(LIBS) $(FLAGS) -o $(APP_DEMO)


# ----
#	lib
# ----
#	make static lib
libstatic: $(OBJECTS)
	ar rvs lib$(LIBNAME).a $(OBJECTS)

#	make dll
libshared: $(OBJECTS) $(LIBNAME).res
	g++ $(OBJECTS) $(LIBNAME).res $(LIBS) $(FLAGS) -s -shared -o $(LIBNAME).dll -Wl,--out-implib,lib$(LIBNAME).dll.a

# ----
#	resources
# ----
$(LIBNAME).res: $(LIBNAME).rc
	windres -i $(LIBNAME).rc --input-format=rc -o $(LIBNAME).res -O coff 


# ----
#	objects
# ----
src/lambda.o: src/lambda.cpp
	g++ -c src/lambda.cpp -o src/lambda.o $(FLAGS)

src/sockets.o: src/sockets.cpp
	g++ -c src/sockets.cpp -o src/sockets.o $(FLAGS)

src/http.o: src/http.cpp
	g++ -c src/http.cpp -o src/http.o $(FLAGS)

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


src/radishdb.o: src/radishdb.cpp
	g++ -c src/radishdb.cpp -o src/radishdb.o $(FLAGS)