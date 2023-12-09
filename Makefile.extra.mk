EXTRA_OBJ_MAIN		=	extra/extra.a
EXTRA_OBJ_DEPS		=	$(CORE_JSON_OBJ) $(CORE_STORAGE_OBJ) $(CORE_COMPRESS_OBJ)

CORE_JSON_OBJ		=	extra/json.a
CORE_JSON_DEPS		=	extra/json/property.o extra/json/parse.o extra/json/stringify.o

CORE_STORAGE_OBJ	=	extra/storage.a
CORE_STORAGE_DEPS	=	extra/storage/storage.o extra/storage/localstorage.o

CORE_COMPRESS_OBJ	=	extra/compression.a
CORE_COMPRESS_DEPS	=	extra/compression/brotli.o extra/compression/zlib.o
CORE_COMPRESS_LIBS	=	-lz -lbrotlicommon -lbrotlidec -lbrotlienc

# extra object
octo.extra: $(EXTRA_OBJ_MAIN)

$(EXTRA_OBJ_MAIN): $(EXTRA_OBJ_DEPS)
	ar rvs $(EXTRA_OBJ_MAIN) $(EXTRA_OBJ_DEPS)

# jisson stuff
$(CORE_JSON_OBJ): $(CORE_JSON_DEPS)
	ar rvs $(CORE_JSON_OBJ) $(CORE_JSON_DEPS)

extra/json/parse.o: extra/json/parse.cpp
	g++ -c $(CFLAGS) extra/json/parse.cpp -o extra/json/parse.o

extra/json/stringify.o: extra/json/stringify.cpp
	g++ -c $(CFLAGS) extra/json/stringify.cpp -o extra/json/stringify.o

extra/json/property.o: extra/json/property.cpp
	g++ -c $(CFLAGS) extra/json/property.cpp -o extra/json/property.o

# storage stuff
$(CORE_STORAGE_OBJ): $(CORE_STORAGE_DEPS)
	ar rvs $(CORE_STORAGE_OBJ) $(CORE_STORAGE_DEPS)

extra/storage/storage.o: extra/storage/storage.cpp
	g++ -c $(CFLAGS) extra/storage/storage.cpp -o extra/storage/storage.o

extra/storage/localstorage.o: extra/storage/localstorage.cpp
	g++ -c $(CFLAGS) extra/storage/localstorage.cpp -o extra/storage/localstorage.o

# compression stuff
$(CORE_COMPRESS_OBJ): $(CORE_COMPRESS_DEPS)
	ar rvs $(CORE_COMPRESS_OBJ) $(CORE_COMPRESS_DEPS)

extra/compression/brotli.o: extra/compression/brotli.cpp
	g++ -c $(CFLAGS) extra/compression/brotli.cpp -o extra/compression/brotli.o $(CORE_COMPRESS_LIBS)

extra/compression/zlib.o: extra/compression/zlib.cpp
	g++ -c $(CFLAGS) extra/compression/zlib.cpp -o extra/compression/zlib.o $(CORE_COMPRESS_LIBS)
