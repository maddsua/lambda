LIB_EXTRA				=	extra/extra.a
LIB_EXTRA_DEPS			=	$(LIB_EXTRA_JSON_DEPS) $(LIB_EXTRA_STORAGE_DEPS)

LIB_EXTRA_JSON			=	extra/json.a
LIB_EXTRA_JSON_DEPS		=	extra/json/property.o extra/json/parse.o extra/json/stringify.o

LIB_EXTRA_STORAGE		=	extra/storage.a
LIB_EXTRA_STORAGE_DEPS	=	extra/storage/storage.o extra/storage/localstorage.o


# extra object
lambda.extra: $(LIB_EXTRA)

$(LIB_EXTRA): $(LIB_EXTRA_DEPS)
	ar rvs $(LIB_EXTRA) $(LIB_EXTRA_DEPS)


# jisson stuff
$(LIB_EXTRA_JSON): $(LIB_EXTRA_JSON_DEPS)
	ar rvs $(LIB_EXTRA_JSON) $(LIB_EXTRA_JSON_DEPS)

extra/json/parse.o: extra/json/parse.cpp
	g++ -c $(CFLAGS) extra/json/parse.cpp -o extra/json/parse.o

extra/json/stringify.o: extra/json/stringify.cpp
	g++ -c $(CFLAGS) extra/json/stringify.cpp -o extra/json/stringify.o

extra/json/property.o: extra/json/property.cpp
	g++ -c $(CFLAGS) extra/json/property.cpp -o extra/json/property.o


# storage stuff
$(LIB_EXTRA_STORAGE): $(LIB_EXTRA_STORAGE_DEPS)
	ar rvs $(LIB_EXTRA_STORAGE) $(LIB_EXTRA_STORAGE_DEPS)

extra/storage/storage.o: extra/storage/storage.cpp
	g++ -c $(CFLAGS) extra/storage/storage.cpp -o extra/storage/storage.o

extra/storage/localstorage.o: extra/storage/localstorage.cpp
	g++ -c $(CFLAGS) extra/storage/localstorage.cpp -o extra/storage/localstorage.o
