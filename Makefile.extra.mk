EXTRA_OBJ_MAIN		=	extra/extra.a
EXTRA_OBJ_DEPS		=	$(CORE_JSON_OBJ) $(CORE_STORAGE_OBJ)

CORE_JSON_OBJ		=	extra/json.a
CORE_JSON_DEPS		=	extra/json/property.o extra/json/parse.o extra/json/stringify.o

CORE_STORAGE_OBJ	=	extra/storage.a
CORE_STORAGE_DEPS	=	extra/storage/storage.o extra/storage/localstorage.o

# extra object
octo.extra: $(EXTRA_OBJ_MAIN)

$(EXTRA_OBJ_MAIN): $(EXTRA_OBJ_DEPS)
	ar rvs $(EXTRA_OBJ_MAIN) $(EXTRA_OBJ_DEPS)

# jisson stuff
$(CORE_JSON_OBJ): $(CORE_JSON_DEPS)
	ar rvs $(CORE_JSON_OBJ) $(CORE_JSON_DEPS)

extra/json/parse.o: extra/json/parse.cpp
	g++ -c extra/json/parse.cpp -o extra/json/parse.o $(CFLAGS)

extra/json/stringify.o: extra/json/stringify.cpp
	g++ -c extra/json/stringify.cpp -o extra/json/stringify.o $(CFLAGS)

extra/json/property.o: extra/json/property.cpp
	g++ -c extra/json/property.cpp -o extra/json/property.o $(CFLAGS)

# storage stuff
$(CORE_STORAGE_OBJ): $(CORE_STORAGE_DEPS)
	ar rvs $(CORE_STORAGE_OBJ) $(CORE_STORAGE_DEPS)

extra/storage/storage.o: extra/storage/storage.cpp
	g++ -c extra/storage/storage.cpp -o extra/storage/storage.o $(CFLAGS)

extra/storage/localstorage.o: extra/storage/localstorage.cpp
	g++ -c extra/storage/localstorage.cpp -o extra/storage/localstorage.o $(CFLAGS)
